#pragma once
#ifndef _MX_VK_DESCRIPTOR_H_
#define _MX_VK_DESCRIPTOR_H_

#include"MxVkComponent.h"

#include<map>
#include<vector>

namespace Mix {
    namespace Vk {
        class DescriptorSetLayout :public Component {
        private:
            VkDescriptorSetLayout mLayout;
            std::vector<VkDescriptorSetLayoutBinding> mBindings;

            void clear() { mBindings.clear(); }
        public:
            DescriptorSetLayout();
            void addBindings(uint32_t binding, VkDescriptorType type, uint32_t count,
                             VkShaderStageFlags stage,
                             const VkSampler* immutableSamplers = nullptr);
            bool createDescriptorSetLayout();
            VkDescriptorSetLayout getDescriptorSetLayout() const { return mLayout; };
            void destroy();
            ~DescriptorSetLayout() { destroy(); }
        };

        class MxVkDescriptorPool :public Component {
        private:
            VkDescriptorPool mDescriptorPool;

            std::map<VkDescriptorType, uint32_t> mPoolSizes;

        public:
            MxVkDescriptorPool();

            void addPoolSize(VkDescriptorType type, uint32_t count);
            bool createDescriptorPool(uint32_t maxSets);
            VkDescriptorPool getDescriptorPool() const { return mDescriptorPool; }

            std::vector<VkDescriptorSet> allocDescriptorSet(const std::vector<VkDescriptorSetLayout>& layout);
            std::vector<VkDescriptorSet> allocDescriptorSet(const VkDescriptorSetLayout layout, const uint32_t count);
            VkDescriptorSet allocDescriptorSet(const VkDescriptorSetLayout layout);

            std::vector<VkDescriptorSet> allocDescriptorSet(const std::vector<DescriptorSetLayout>& layout);
            std::vector<VkDescriptorSet> allocDescriptorSet(const DescriptorSetLayout& layout, const uint32_t count);;
            VkDescriptorSet allocDescriptorSet(const DescriptorSetLayout& layout);
            ~MxVkDescriptorPool();
            void destroy();
        };
    }
}
#endif // !_MX_VK_DESCRIPTOR_H_
