﻿#pragma once

#include "Graphics/Images/Image2d.hpp"
#include "Swapchain.hpp"

namespace acid {
class ImageDepth;
class Renderpass;
class RenderStage;

class ACID_EXPORT Framebuffers : public virtual NonCopyable {
public:
	Framebuffers(const Vector2ui &extent, const RenderStage &renderStage, const Renderpass &renderPass, const Swapchain &swapchain, const ImageDepth &depthStencil,
		const VkSampleCountFlagBits &samples = VK_SAMPLE_COUNT_1_BIT);
	~Framebuffers();

	Image2d *GetAttachment(uint32_t index) const { return m_imageAttachments[index].get(); }

	const std::vector<std::unique_ptr<Image2d>> &GetImageAttachments() const { return m_imageAttachments; }
	const std::vector<VkFramebuffer> &GetFramebuffers() const { return m_framebuffers; }

private:
	std::vector<std::unique_ptr<Image2d>> m_imageAttachments;
	std::vector<VkFramebuffer> m_framebuffers;
};
}
