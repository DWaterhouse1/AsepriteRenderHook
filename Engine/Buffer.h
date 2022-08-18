#pragma once

#include "Device.h"

namespace wrengine {

/**
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */
class Buffer {
public:
  Buffer(
    Device& device,
    VkDeviceSize instanceSize,
    uint32_t instanceCount,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize minOffsetAlignment = 1);
  ~Buffer();

  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  void unmap();

  // operations on buffer memory ranges
  // methods returning VkResult type can fail
  void writeToBuffer(
    void* data,
    VkDeviceSize size = VK_WHOLE_SIZE,
    VkDeviceSize offset = 0);
  VkResult flush(
    VkDeviceSize size = VK_WHOLE_SIZE,
    VkDeviceSize offset = 0);
  VkDescriptorBufferInfo descriptorInfo(
    VkDeviceSize size = VK_WHOLE_SIZE,
    VkDeviceSize offset = 0);
  VkResult invalidate(
    VkDeviceSize size = VK_WHOLE_SIZE,
    VkDeviceSize offset = 0);

  // operations on buffers by indices
  // methods returning VkResult type can fail
  void writeToIndex(void* data, int index);
  VkResult flushIndex(int index);
  VkDescriptorBufferInfo descriptorInfoForIndex(int index);
  VkResult invalidateIndex(int index);

  // getters
  VkBuffer getBuffer() const { return m_buffer; }
  void* getMappedMemory() const { return m_mapped; }
  uint32_t getInstanceCount() const { return m_instanceCount; }
  VkDeviceSize getInstanceSize() const { return m_instanceSize; }
  VkDeviceSize getAlignmentSize() const { return m_alignmentSize; }
  VkBufferUsageFlags getUsageFlags() const { return m_usageFlags; }
  VkDeviceSize getBufferSize() const { return m_bufferSize; }
  VkMemoryPropertyFlags getMemoryPropertyFlags() const
  { 
    return m_memoryPropertyFlags;
  }

private:
  static VkDeviceSize getAlignment(
    VkDeviceSize instanceSize,
    VkDeviceSize minOffsetAlignment);

  Device& m_device;
  void* m_mapped = nullptr;
  VkBuffer m_buffer = VK_NULL_HANDLE;
  VkDeviceMemory m_memory = VK_NULL_HANDLE;

  VkDeviceSize m_bufferSize;
  uint32_t m_instanceCount;
  VkDeviceSize m_instanceSize;
  VkDeviceSize m_alignmentSize;
  VkBufferUsageFlags m_usageFlags;
  VkMemoryPropertyFlags m_memoryPropertyFlags;
};
}  // namespace lve