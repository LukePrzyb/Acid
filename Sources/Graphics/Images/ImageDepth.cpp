#include "ImageDepth.hpp"

#include "Graphics/Buffers/Buffer.hpp"
#include "Graphics/Graphics.hpp"

namespace acid {
static const std::vector<VkFormat> TRY_FORMATS = {
	VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
	VK_FORMAT_D16_UNORM
};

ImageDepth::ImageDepth(const Vector2ui &extent, VkSampleCountFlagBits samples) :
	m_extent(extent) {
	auto physicalDevice = Graphics::Get()->GetPhysicalDevice();

	m_format = Image::FindSupportedFormat(TRY_FORMATS, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	if (m_format == VK_FORMAT_UNDEFINED) {
		throw std::runtime_error("No depth stencil format could be selected");
	}

	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	if (Image::HasStencil(m_format)) {
		aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	Image::CreateImage(m_image, m_memory, {m_extent.m_x, m_extent.m_y, 1}, m_format, samples, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 1, VK_IMAGE_TYPE_2D);
	Image::CreateImageSampler(m_sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false, 1);
	Image::CreateImageView(m_image, m_view, VK_IMAGE_VIEW_TYPE_2D, m_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 0, 1, 0);
	Image::TransitionImageLayout(m_image, m_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, aspectMask, 1, 0, 1, 0);
}

ImageDepth::~ImageDepth() {
	auto logicalDevice = Graphics::Get()->GetLogicalDevice();

	vkDestroyImageView(*logicalDevice, m_view, nullptr);
	vkDestroySampler(*logicalDevice, m_sampler, nullptr);
	vkFreeMemory(*logicalDevice, m_memory, nullptr);
	vkDestroyImage(*logicalDevice, m_image, nullptr);
}

WriteDescriptorSet ImageDepth::GetWriteDescriptor(uint32_t binding, VkDescriptorType descriptorType, const std::optional<OffsetSize> &offsetSize) const {
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.sampler = m_sampler;
	imageInfo.imageView = m_view;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = VK_NULL_HANDLE; // Will be set in the descriptor handler.
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.descriptorType = descriptorType;
	//descriptorWrite.pImageInfo = &imageInfo;
	return {descriptorWrite, imageInfo};
}

VkDescriptorSetLayoutBinding ImageDepth::GetDescriptorSetLayout(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stage) {
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
	descriptorSetLayoutBinding.binding = binding;
	descriptorSetLayoutBinding.descriptorType = descriptorType;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = stage;
	descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
	return descriptorSetLayoutBinding;
}
}
