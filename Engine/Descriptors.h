#pragma once

#include "Device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

/**
 * Abstraction over Vulkan descriptor sets.
 *
 * DescriptorSetLayout and Discriptor pool are RAII encapsulations of these
 * vulkan structures, and each are provided with a builder class for config.
 *
 * DescriptorWriters is a builder class for constructing and executing
 * descriptor writes.
 */

namespace wrengine
{

/**
* Container for descriptor set layout and bindings. Defines a builder class to
* provide easier interface for construction.
* 
*/
class DescriptorSetLayout
{
	typedef std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> BindingMap;

public:
	/**
	* Builder class for the descriptor set layout. Provides interface to define
	* binding types, and will return a configured DescriptorSetLayout object on
	* build.
	*/
	class Builder
	{
	public:
		Builder(Device& device) : m_device{ device } {}

		Builder& addBinding(
			uint32_t binding,
			VkDescriptorType descriptorType,
			VkShaderStageFlags stageFlags,
			uint32_t count = 1);
		std::unique_ptr<DescriptorSetLayout> build() const;

	private:
		Device& m_device;
		BindingMap m_bindings{};
	};

	DescriptorSetLayout(
		Device& device,
		BindingMap bindings);
	~DescriptorSetLayout();

	// not copyable
	DescriptorSetLayout(const DescriptorSetLayout&) = delete;
	DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

	// getters
	VkDescriptorSetLayout getDescriptorSetLayout() const
	{ return m_descriptorSetLayout; }

private:
	Device& m_device;
	VkDescriptorSetLayout m_descriptorSetLayout;
	BindingMap m_bindings;

	friend class DescriptorWriter;
};

/**
* Container for a descriptor pool. Defines a builder class to provide easier
* interface for construction.
*
*/
class DescriptorPool
{
public:
	/**
	* Builder class for the descriptor pool. Provides interface to define binding
	* types, and will return a configured DescriptorPool object on build.
	*/
	class Builder
	{
	public:
		Builder(Device& device) : m_device{ device } {}

		// pool config and build methods
		Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
		Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
		Builder& setMaxSets(uint32_t count);
		std::unique_ptr<DescriptorPool> build() const;

	private:
		Device& m_device;
		std::vector<VkDescriptorPoolSize> m_poolSizes{};
		uint32_t m_maxSets = 1000;
		VkDescriptorPoolCreateFlags m_poolFlags = 0;
	};

	DescriptorPool(
		Device& device,
		uint32_t maxSets,
		VkDescriptorPoolCreateFlags poolFlags,
		const std::vector<VkDescriptorPoolSize>& poolSizes
	);
	~DescriptorPool();

	// not copyable
	DescriptorPool(const DescriptorPool&) = delete;
	DescriptorPool& operator=(const DescriptorPool&) = delete;

	bool allocateDescriptor(
		const VkDescriptorSetLayout descriptorSetlayout,
		VkDescriptorSet& descriptor) const;

	void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

	void resetPool();

private:
	Device& m_device;
	VkDescriptorPool m_descriptorPool;

	friend class DescriptorWriter;
};

/**
* Builder class to define and execute a sequence of descriptor writes.
*/
class DescriptorWriter
{
public:
	DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

	DescriptorWriter& writeBuffer(
		uint32_t binding,
		VkDescriptorBufferInfo* bufferInfo);

	DescriptorWriter& writeImage(
		uint32_t binding,
		VkDescriptorImageInfo* imageInfo);

	bool build(VkDescriptorSet& set);
	void overwrite(VkDescriptorSet& set);

private:
	DescriptorSetLayout& m_descriptorSetLayout;
	DescriptorPool& m_pool;
	std::vector<VkWriteDescriptorSet> m_writes;
};
}