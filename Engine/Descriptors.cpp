#include "Descriptors.h"

// std
#include <cassert>
#include <stdexcept>

namespace wrengine
{
// ------------------ Descriptor Set Layout Builder ------------------

/**
* Appends a descriptor binding type to the map of bindings currently held by
* the builder class.
* 
* @param binding Index of the binding to add.
* @param descriptorType Type of the descriptor (eg uniform or storage buffer).
* @param stageFlags Flags configuring shader access to the binding.
* @param count (Optional) Size of the desciptors array to be bound.
* 
* @return Reference to the builder object.
*/
DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(
	uint32_t binding,
	VkDescriptorType descriptorType,
	VkShaderStageFlags stageFlags,
	uint32_t count
)
{
	assert(m_bindings.count(binding) == 0 && "binding already in use");
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = descriptorType;
	layoutBinding.stageFlags = stageFlags;
	layoutBinding.descriptorCount = count;
	m_bindings[binding] = layoutBinding;
	return *this;
}

/**
* Builds the descriptor set layout using configured bindings.
* 
* @return Unique pointer to the built DescriptorSetLayout
*/
std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const
{
	return std::make_unique<DescriptorSetLayout>(m_device, m_bindings);
}

// ---------------------- Descriptor Set Layout ----------------------

DescriptorSetLayout::DescriptorSetLayout(Device& device, BindingMap bindings) :
	m_device{ device },
	m_bindings{ bindings }
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
	for (auto [binding, descriptorSetLayoutBinding] : bindings)
	{
		setLayoutBindings.push_back(descriptorSetLayoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType =
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount =
		static_cast<uint32_t>(setLayoutBindings.size());
	descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

	if (vkCreateDescriptorSetLayout(
		m_device.device(),
		&descriptorSetLayoutInfo,
		nullptr,
		&m_descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(
		m_device.device(),
		m_descriptorSetLayout,
		nullptr);
}

// --------------------- Descriptor Pool Builder----------------------

/**
* Adds a pool size paramater of a given type to the Descriptor Pool build
* configuration.
* 
* @param descriptorType The type of the Descriptor Pool size to add.
* @param count The size of the new Descriptor Pool type.
* 
* @return Reference to the builder object.
*/
DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(
	VkDescriptorType descriptorType,
	uint32_t count)
{
	m_poolSizes.push_back({ descriptorType, count });
	return *this;
}

/**
* Sets the pool creation flags for the Descriptor Pool bujild configuration
* 
* @param flags The descriptor pool create flags to add.
* 
* @return Reference to the builder object.
*/
DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(
	VkDescriptorPoolCreateFlags flags)
{
	m_poolFlags = flags;
	return *this;
}

/**
* Sets the maximum number of descriptors that can be allocated from the pool
* object to be built.
* 
* @param count Max number of descriptor allocations
* 
* @return Reference to the builder object.
*/
DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t count)
{
	m_maxSets = count;
	return *this;
}

/**
* Builds the descriptor pool using configured bindings.
*
* @return Unique pointer to the built DescriptorPool
*/
std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const
{
	return std::make_unique<DescriptorPool>(
		m_device,
		m_maxSets,
		m_poolFlags,
		m_poolSizes);
}

// ------------------------- Descriptor Pool -------------------------

DescriptorPool::DescriptorPool(Device& device,
	uint32_t maxSets,
	VkDescriptorPoolCreateFlags poolFlags,
	const std::vector<VkDescriptorPoolSize>& poolSizes) :
	m_device{ device }
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = maxSets;
	descriptorPoolInfo.flags = poolFlags;

	if (vkCreateDescriptorPool(m_device.device(),
		&descriptorPoolInfo,
		nullptr,
		&m_descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(m_device.device(), m_descriptorPool, nullptr);
}

/**
* Allocates a descriptor set from the descriptor pool
* 
* @param descriptorSetLayout The Layout of the pool from which the descriptor
* set will be allocated.
* @param descriptor VkDescriptorSet to be created, passed by reference.
* 
* @return Boolean return to indicate alloc success of failure.
*/
bool DescriptorPool::allocateDescriptor(
	const VkDescriptorSetLayout descriptorSetlayout,
	VkDescriptorSet& descriptor) const
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.pSetLayouts = &descriptorSetlayout;
	allocInfo.descriptorSetCount = 1;

	if (vkAllocateDescriptorSets(m_device.device(), &allocInfo, &descriptor)
		!= VK_SUCCESS)
	{
		return false;
	}
	return true;
}

/**
* Frees a descriptor sets from a provided list.
* 
* @param descriptors Vector of all descriptors to be freed.
*/
void DescriptorPool::freeDescriptors(
	std::vector<VkDescriptorSet>& descriptors) const
{
	vkFreeDescriptorSets(
		m_device.device(),
		m_descriptorPool,
		static_cast<uint32_t>(descriptors.size()),
		descriptors.data());
}

/**
* Resets the entire descriptor pool.
*/
void DescriptorPool::resetPool()
{
	vkResetDescriptorPool(m_device.device(), m_descriptorPool, 0);
}

// ------------------------ Descriptor Writer ------------------------

DescriptorWriter::DescriptorWriter(
	DescriptorSetLayout& setLayout,
	DescriptorPool& pool) :
	m_descriptorSetLayout{ setLayout },
	m_pool{ pool }
{}

/**
* Defines a buffer write to be executed on build of the DescriptorWriter object.
* 
* @param binding The binding location of the descriptor into which the buffer
* info will be written.
* @param bufferInfo Ptr to structure describing the buffer info to write.
*/
DescriptorWriter& DescriptorWriter::writeBuffer(
	uint32_t binding,
	VkDescriptorBufferInfo* bufferInfo)
{
	assert(
		m_descriptorSetLayout.m_bindings.count(binding) == 1 &&
		"layout does not contain specified binding");
	VkDescriptorSetLayoutBinding& bindingDescription =
		m_descriptorSetLayout.m_bindings[binding];

	assert(
		bindingDescription.descriptorCount == 1 &&
		"binding single descriptor info, but binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pBufferInfo = bufferInfo;
	write.descriptorCount = 1;

	m_writes.push_back(write);
	return *this;
}

/**
* Defines an image write to be executed on build of the DescriptorWriter object.
*
* @param binding The binding location of the descriptor into which the image
* info will be written.
* @param bufferInfo Ptr to structure describing the image info to write.
*/
DescriptorWriter& DescriptorWriter::writeImage(
	uint32_t binding,
	VkDescriptorImageInfo* imageInfo)
{
	assert(
		m_descriptorSetLayout.m_bindings.count(binding) == 1 &&
		"layout does not contain specified binding");
	VkDescriptorSetLayoutBinding& bindingDescription =
		m_descriptorSetLayout.m_bindings[binding];

	assert(
		bindingDescription.descriptorCount == 1 &&
		"binding single descriptor info, but binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pImageInfo = imageInfo;
	write.descriptorCount = 1;

	m_writes.push_back(write);
	return *this;
}

/**
* Executes the all the descriptor writes that the builder object has been
* configured with.
* 
* @param set The descriptor set which will be written to, passed by reference.
* 
* @return Boolean value indicating pool allocation success or failure.
*/
bool DescriptorWriter::build(VkDescriptorSet& set)
{
	bool success = m_pool.allocateDescriptor(
		m_descriptorSetLayout.getDescriptorSetLayout(),
		set);

	if (!success) return false;
	overwrite(set);
	return true;
}

/**
* Overwites existing descriptor writes using all such writes that the builder
* object has been configured with.
*
* @param set The descriptor set which will be written to, passed by reference.
*
*/
void DescriptorWriter::overwrite(VkDescriptorSet& set)
{
	for (auto& write : m_writes)
	{
		write.dstSet = set;
	}
	vkUpdateDescriptorSets(
		m_pool.m_device.device(),
		static_cast<uint32_t>(m_writes.size()),
		m_writes.data(),
		0,
		nullptr);
}
} // namespace wrengine