#include "Graphics.hpp"

#include <SPIRV/GlslangToSpv.h>
#include "Devices/Window.hpp"
#include "Subrender.hpp"

namespace acid {
Graphics::Graphics() :
	m_elapsedPurge(5s),
	m_instance(std::make_unique<Instance>()),
	m_physicalDevice(std::make_unique<PhysicalDevice>(m_instance.get())),
	m_surface(std::make_unique<Surface>(m_instance.get(), m_physicalDevice.get())),
	m_logicalDevice(std::make_unique<LogicalDevice>(m_instance.get(), m_physicalDevice.get(), m_surface.get())) {
	glslang::InitializeProcess();

	CreatePipelineCache();
}

Graphics::~Graphics() {
	auto graphicsQueue = m_logicalDevice->GetGraphicsQueue();

	CheckVk(vkQueueWaitIdle(graphicsQueue));

	glslang::FinalizeProcess();

	vkDestroyPipelineCache(*m_logicalDevice, m_pipelineCache, nullptr);

	for (std::size_t i = 0; i < m_flightFences.size(); i++) {
		vkDestroyFence(*m_logicalDevice, m_flightFences[i], nullptr);
		vkDestroySemaphore(*m_logicalDevice, m_renderCompletes[i], nullptr);
		vkDestroySemaphore(*m_logicalDevice, m_presentCompletes[i], nullptr);
	}
}

void Graphics::Update() {
	if (!m_renderer || Window::Get()->IsIconified()) {
		return;
	}

	if (!m_renderer->m_started) {
		ResetRenderStages();
		m_renderer->Start();
		m_renderer->m_started = true;
	}

	m_renderer->Update();

	auto acquireResult = m_swapchain->AcquireNextImage(m_presentCompletes[m_currentFrame], m_flightFences[m_currentFrame]);

	if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapchain();
		return;
	}

	if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
		Log::Error("Failed to acquire swap chain image!\n");
		return;
	}

	Pipeline::Stage stage;

	for (auto &renderStage : m_renderer->m_renderStages) {
		renderStage->Update();

		if (!StartRenderpass(*renderStage)) {
			return;
		}

		auto &commandBuffer = m_commandBuffers[m_swapchain->GetActiveImageIndex()];
		
		for (const auto &subpass : renderStage->GetSubpasses()) {
			stage.second = subpass.GetBinding();

			// Renders subpass subrender pipelines.
			m_renderer->m_subrenderHolder.RenderStage(stage, *commandBuffer);

			if (subpass.GetBinding() != renderStage->GetSubpasses().back().GetBinding()) {
				vkCmdNextSubpass(*commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
			}
		}

		EndRenderpass(*renderStage);
		stage.first++;
	}

	// Purges unused command pools.
	if (m_elapsedPurge.GetElapsed() != 0) {
		for (auto it = m_commandPools.begin(); it != m_commandPools.end();) {
			if ((*it).second.use_count() <= 1) {
				it = m_commandPools.erase(it);
				continue;
			}

			++it;
		}
	}
}

std::string Graphics::StringifyResultVk(const VkResult &result) {
	switch (result) {
	case VK_SUCCESS:
		return "Success";
	case VK_NOT_READY:
		return "A fence or query has not yet completed";
	case VK_TIMEOUT:
		return "A wait operation has not completed in the specified time";
	case VK_EVENT_SET:
		return "An event is signaled";
	case VK_EVENT_RESET:
		return "An event is unsignaled";
	case VK_INCOMPLETE:
		return "A return array was too small for the result";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "A host memory allocation has failed";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "A device memory allocation has failed";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "Initialization of an object could not be completed for implementation-specific reasons";
	case VK_ERROR_DEVICE_LOST:
		return "The logical or physical device has been lost";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "Mapping of a memory object has failed";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "A requested layer is not present or could not be loaded";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "A requested extension is not supported";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "A requested feature is not supported";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "Too many objects of the type have already been created";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "A requested format is not supported on this device";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "A surface is no longer available";
		//case VK_ERROR_OUT_OF_POOL_MEMORY:
		//	return "A allocation failed due to having no more space in the descriptor pool";
	case VK_SUBOPTIMAL_KHR:
		return "A swapchain no longer matches the surface properties exactly, but can still be used";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "A surface has changed in such a way that it is no longer compatible with the swapchain";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "The display used by a swapchain does not use the same presentable image layout";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "A validation layer found an error";
	default:
		return "Unknown Vulkan error";
	}
}

void Graphics::CheckVk(const VkResult &result) {
	if (result >= 0) {
		return;
	}

	auto failure = StringifyResultVk(result);

	throw std::runtime_error("Vulkan error: " + failure);
}

void Graphics::CaptureScreenshot(const std::filesystem::path &filename) const {
#if defined(ACID_DEBUG)
	auto debugStart = Time::Now();
#endif

	auto size = Window::Get()->GetSize();

	VkImage dstImage;
	VkDeviceMemory dstImageMemory;
	auto supportsBlit = Image::CopyImage(m_swapchain->GetActiveImage(), dstImage, dstImageMemory, m_surface->GetFormat().format, {size.m_x, size.m_y, 1},
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0, 0);

	// Get layout of the image (including row pitch).
	VkImageSubresource imageSubresource = {};
	imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresource.mipLevel = 0;
	imageSubresource.arrayLayer = 0;

	VkSubresourceLayout dstSubresourceLayout;
	vkGetImageSubresourceLayout(*m_logicalDevice, dstImage, &imageSubresource, &dstSubresourceLayout);

	Bitmap bitmap(std::make_unique<uint8_t[]>(dstSubresourceLayout.size), size);

	void *data;
	vkMapMemory(*m_logicalDevice, dstImageMemory, dstSubresourceLayout.offset, dstSubresourceLayout.size, 0, &data);
	std::memcpy(bitmap.GetData().get(), data, static_cast<size_t>(dstSubresourceLayout.size));
	vkUnmapMemory(*m_logicalDevice, dstImageMemory);

	// Frees temp image and memory.
	vkFreeMemory(*m_logicalDevice, dstImageMemory, nullptr);
	vkDestroyImage(*m_logicalDevice, dstImage, nullptr);

	// Writes the screenshot bitmap to the file.
	bitmap.Write(filename);

#if defined(ACID_DEBUG)
	Log::Out("Screenshot ", filename, " created in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}

RenderStage *Graphics::GetRenderStage(uint32_t index) const {
	if (!m_renderer)
		return nullptr;
	return m_renderer->GetRenderStage(index);
}

const Descriptor *Graphics::GetAttachment(const std::string &name) const {
	auto it = m_attachments.find(name);

	if (it == m_attachments.end()) {
		return nullptr;
	}

	return it->second;
}

const std::shared_ptr<CommandPool> &Graphics::GetCommandPool(const std::thread::id &threadId) {
	auto it = m_commandPools.find(threadId);

	if (it != m_commandPools.end()) {
		return it->second;
	}

	// TODO: Cleanup and fix crashes
	return m_commandPools.emplace(threadId, std::make_shared<CommandPool>(threadId)).first->second;
}

void Graphics::CreatePipelineCache() {
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	CheckVk(vkCreatePipelineCache(*m_logicalDevice, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache));
}

void Graphics::ResetRenderStages() {
	RecreateSwapchain();

	if (m_flightFences.size() != m_swapchain->GetImageCount()) {
		RecreateCommandBuffers();
	}

	for (const auto &renderStage : m_renderer->m_renderStages) {
		renderStage->Rebuild(*m_swapchain);
	}

	RecreateAttachmentsMap();
}

void Graphics::RecreateSwapchain() {
	vkDeviceWaitIdle(*m_logicalDevice);

	VkExtent2D displayExtent = {Window::Get()->GetSize().m_x, Window::Get()->GetSize().m_y};
#if defined(ACID_DEBUG)
	if (m_swapchain)
		Log::Out("Recreating swapchain old (", m_swapchain->GetExtent().width, ", ", m_swapchain->GetExtent().height, ") new (", displayExtent.width, ", ", displayExtent.height, ")\n");
#endif
	m_swapchain = std::make_unique<Swapchain>(displayExtent, m_swapchain.get());
	RecreateCommandBuffers();
}

void Graphics::RecreateCommandBuffers() {
	for (std::size_t i = 0; i < m_flightFences.size(); i++) {
		vkDestroyFence(*m_logicalDevice, m_flightFences[i], nullptr);
		vkDestroySemaphore(*m_logicalDevice, m_renderCompletes[i], nullptr);
		vkDestroySemaphore(*m_logicalDevice, m_presentCompletes[i], nullptr);
	}

	m_presentCompletes.resize(m_swapchain->GetImageCount());
	m_renderCompletes.resize(m_swapchain->GetImageCount());
	m_flightFences.resize(m_swapchain->GetImageCount());
	m_commandBuffers.resize(m_swapchain->GetImageCount());

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (std::size_t i = 0; i < m_flightFences.size(); i++) {
		CheckVk(vkCreateSemaphore(*m_logicalDevice, &semaphoreCreateInfo, nullptr, &m_presentCompletes[i]));

		CheckVk(vkCreateSemaphore(*m_logicalDevice, &semaphoreCreateInfo, nullptr, &m_renderCompletes[i]));

		CheckVk(vkCreateFence(*m_logicalDevice, &fenceCreateInfo, nullptr, &m_flightFences[i]));

		m_commandBuffers[i] = std::make_unique<CommandBuffer>(false);
	}
}

void Graphics::RecreatePass(RenderStage &renderStage) {
	auto graphicsQueue = m_logicalDevice->GetGraphicsQueue();

	VkExtent2D displayExtent = {Window::Get()->GetSize().m_x, Window::Get()->GetSize().m_y};

	CheckVk(vkQueueWaitIdle(graphicsQueue));

	if (renderStage.HasSwapchain() && (m_framebufferResized || !m_swapchain->IsSameExtent(displayExtent))) {
		RecreateSwapchain();
	}

	renderStage.Rebuild(*m_swapchain);
	RecreateAttachmentsMap(); // TODO: Maybe not recreate on a single change.
}

void Graphics::RecreateAttachmentsMap() {
	m_attachments.clear();

	for (const auto &renderStage : m_renderer->m_renderStages) {
		m_attachments.insert(renderStage->m_descriptors.begin(), renderStage->m_descriptors.end());
	}
}

bool Graphics::StartRenderpass(RenderStage &renderStage) {
	if (renderStage.IsOutOfDate()) {
		RecreatePass(renderStage);
		return false;
	}

	auto &commandBuffer = m_commandBuffers[m_swapchain->GetActiveImageIndex()];
	
	if (!commandBuffer->IsRunning())
		commandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

	VkRect2D renderArea = {};
	renderArea.offset = {renderStage.GetRenderArea().GetOffset().m_x, renderStage.GetRenderArea().GetOffset().m_y};
	renderArea.extent = {renderStage.GetRenderArea().GetExtent().m_x, renderStage.GetRenderArea().GetExtent().m_y};

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(renderArea.extent.width);
	viewport.height = static_cast<float>(renderArea.extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = renderArea.offset;
	scissor.extent = renderArea.extent;
	vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);

	auto clearValues = renderStage.GetClearValues();

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = *renderStage.GetRenderpass();
	renderPassBeginInfo.framebuffer = renderStage.GetActiveFramebuffer(m_swapchain->GetActiveImageIndex());
	renderPassBeginInfo.renderArea = renderArea;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();
	vkCmdBeginRenderPass(*commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	return true;
}

void Graphics::EndRenderpass(RenderStage &renderStage) {
	auto presentQueue = m_logicalDevice->GetPresentQueue();
	auto &commandBuffer = m_commandBuffers[m_swapchain->GetActiveImageIndex()];

	vkCmdEndRenderPass(*commandBuffer);

	if (!renderStage.HasSwapchain())
		return;

	commandBuffer->End();
	commandBuffer->Submit(m_presentCompletes[m_currentFrame], m_renderCompletes[m_currentFrame], m_flightFences[m_currentFrame]);

	auto presentResult = m_swapchain->QueuePresent(presentQueue, m_renderCompletes[m_currentFrame]);
	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) { // || m_framebufferResized
		m_framebufferResized = true; // false
		//RecreateSwapchain();
	} else if (presentResult != VK_SUCCESS) {
		CheckVk(presentResult);
		Log::Error("Failed to present swap chain image!\n");
	}

	m_currentFrame = (m_currentFrame + 1) % m_swapchain->GetImageCount();
}
}
