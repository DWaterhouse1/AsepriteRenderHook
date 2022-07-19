#pragma once

#include "Device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace wrengine
{
	class DescriptorSetLayout
	{
		typedef std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> BindingMap;

	public:
		class Builder
		{
		public:
			Builder(Device& device) : m_device{ device } {}

			Builder& addBinding(
				uint32_t binding,
				VkDescriptorType descriptorType,
				VkShaderStageFlags stageFlags,
				uint32_t count = 1
			);
			std::unique_ptr<DescriptorSetLayout> build() const;

		private:
			Device& m_device;
			BindingMap m_bindings{};
		};

		DescriptorSetLayout(
			Device& device,
			BindingMap bindings
		);
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

	class DescriptorPool
	{
	public:
		class Builder
		{
		public:
			Builder(Device& device) : m_device{ device } {}

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
			VkDescriptorSet& descriptor
		) const;

		void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

		void resetPool();

	private:
		Device& m_device;
		VkDescriptorPool m_descriptorPool;

		friend class DescriptorWriter;
	};

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