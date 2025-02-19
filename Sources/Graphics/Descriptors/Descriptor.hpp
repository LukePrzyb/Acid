#pragma once

#include <vulkan/vulkan.h>
#include "StdAfx.hpp"

namespace acid {
class ACID_EXPORT OffsetSize {
public:
	OffsetSize(uint32_t offset, uint32_t size) :
		m_offset(offset),
		m_size(size) {
	}

	uint32_t GetOffset() const { return m_offset; }
	uint32_t GetSize() const { return m_size; }

	bool operator==(const OffsetSize &other) const {
		return m_offset == other.m_offset && m_size == other.m_size;
	}

	bool operator!=(const OffsetSize &other) const {
		return !operator==(other);
	}

private:
	uint32_t m_offset;
	uint32_t m_size;
};

class ACID_EXPORT WriteDescriptorSet {
public:
	WriteDescriptorSet(const VkWriteDescriptorSet &writeDescriptorSet, const VkDescriptorImageInfo &imageInfo) :
		m_writeDescriptorSet(writeDescriptorSet),
		m_imageInfo(std::make_unique<VkDescriptorImageInfo>(imageInfo)) {
		m_writeDescriptorSet.pImageInfo = m_imageInfo.get();
	}

	WriteDescriptorSet(const VkWriteDescriptorSet &writeDescriptorSet, const VkDescriptorBufferInfo &bufferInfo) :
		m_writeDescriptorSet(writeDescriptorSet),
		m_bufferInfo(std::make_unique<VkDescriptorBufferInfo>(bufferInfo)) {
		m_writeDescriptorSet.pBufferInfo = m_bufferInfo.get();
	}

	const VkWriteDescriptorSet &GetWriteDescriptorSet() const { return m_writeDescriptorSet; }

private:
	VkWriteDescriptorSet m_writeDescriptorSet;
	std::unique_ptr<VkDescriptorImageInfo> m_imageInfo;
	std::unique_ptr<VkDescriptorBufferInfo> m_bufferInfo;
};

class ACID_EXPORT Descriptor {
public:
	Descriptor() = default;
	
	virtual ~Descriptor() = default;

	virtual WriteDescriptorSet GetWriteDescriptor(uint32_t binding, VkDescriptorType descriptorType, const std::optional<OffsetSize> &offsetSize) const = 0;
};
}
