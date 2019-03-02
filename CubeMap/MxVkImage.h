#pragma once
#ifndef _MX_VK_IMAGE_H_
#define _MX_VK_IMAGE_H_

#include"MxVkManager.h"

namespace Mix {
    namespace Vk {
        struct Image {
            VkImage image;
            VkImageView view;
            VkDeviceMemory memory;
            VkFormat format;
            VkExtent2D extent;

            static VkImage createImage2D(const Manager* manager,
                                         const VkExtent2D extent,
                                         const VkFormat format,
                                         const VkImageUsageFlags usage,
                                         const VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
                                         const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                                         const VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                         const VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                         const uint32_t mipLevels = 1, const uint32_t arrayLayers = 1);

            static VkImageView createImageView2D(const Manager* manager,
                                                 const VkImage image,
                                                 const VkFormat format,
                                                 const VkImageAspectFlags aspectFlags,
                                                 const uint32_t mipLevel = 0,
                                                 const uint32_t levelCount = 1,
                                                 const uint32_t layer = 0,
                                                 const uint32_t layerCount = 1);

            static VkDeviceMemory allocateImageMemory(const Manager* manager, const VkImage image,
                                                      const VkMemoryPropertyFlags properties);

            static void setImageLayout(VkCommandBuffer cmdbuffer,
                                       VkImage image,
                                       VkImageLayout oldImageLayout,
                                       VkImageLayout newImageLayout,
                                       VkImageSubresourceRange subresourceRange,
                                       VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                       VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

            static Image* createDepthStencil(const Manager* manager,
                                             const VkFormat format, const VkExtent2D& extent, const VkSampleCountFlagBits sampleCount);
        };

        struct Texture2D :public Image {
            VkImageLayout layout;
            uint32_t mipLevels;
        };
    }
}
#endif // !_MX_VK_IMAGE_H_
