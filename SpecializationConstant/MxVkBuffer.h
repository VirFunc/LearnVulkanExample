#pragma once
#ifndef _MX_VK_BUFFER_H_
#define _MX_VK_BUFFER_H_

#include"MxVkManager.h"
#include"MxVkCommandPool.h"

#include<cassert>

namespace Mix {
    namespace Vk {
        struct Buffer {
            const Manager* manager;
            VkBuffer buffer;
            VkDeviceMemory memory;
            VkDescriptorBufferInfo descriptor;
            VkDeviceSize size;
            VkDeviceSize alignment;
            void* mapped;

            VkBufferUsageFlags usages;
            VkMemoryPropertyFlags memoryProperty;

            VkResult map(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);
            void unmap();
            VkResult bind(const VkDeviceSize offset = 0);
            void setupDescriptor(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);
            void copyTo(const void * data, const VkDeviceSize size);
            VkResult flush(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);
            VkResult invalidate(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);
            void destory();

            static Buffer* createBuffer(const Manager* manager, const VkBufferUsageFlags usage,
                                        const VkMemoryPropertyFlags memoryProperty,
                                        const VkDeviceSize size,
                                        const VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE, const void * data = nullptr);

            static void copyToDeviceBuffer(const Manager* manager, CommandPool* commandPool, const Buffer* dstBuffer, const void* data);
            ~Buffer() { destory(); };
        };
    }

}
#endif // !_MX_VK_BUFFER_

