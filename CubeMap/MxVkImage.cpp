#include "MxVkImage.h"

namespace Mix {
    namespace Vk {
        VkImage Image::createImage2D(const Manager * manager, const VkExtent2D extent, const VkFormat format, const VkImageUsageFlags usage, const VkSampleCountFlagBits sampleCount, const VkImageTiling tiling, const VkImageLayout initialLayout, const VkSharingMode sharingMode, const uint32_t mipLevels, const uint32_t arrayLayers) {
            VkImageCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            createInfo.imageType = VK_IMAGE_TYPE_2D;
            createInfo.extent.width = extent.width;
            createInfo.extent.height = extent.height;
            createInfo.extent.depth = 1;
            createInfo.mipLevels = mipLevels;
            createInfo.arrayLayers = arrayLayers;
            createInfo.format = format;
            createInfo.tiling = tiling;
            createInfo.initialLayout = initialLayout;
            createInfo.usage = usage;
            createInfo.sharingMode = sharingMode;
            createInfo.samples = sampleCount;

            VkImage tempImage;
            MX_VK_CHECK_RESULT(vkCreateImage(manager->getDevice(), &createInfo, nullptr, &tempImage));
            return tempImage;
        }

        VkImageView Image::createImageView2D(const Manager* manager, const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags, const uint32_t mipLevel, const uint32_t levelCount, const uint32_t layer, const uint32_t layerCount) {
            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = mipLevel; //the first mipmap level accessible to the view
            viewInfo.subresourceRange.levelCount = levelCount; //the number of mipmap levels (starting from baseMipLevel) accessible to the view
            viewInfo.subresourceRange.baseArrayLayer = layer;
            viewInfo.subresourceRange.layerCount = layerCount;

            VkImageView imageView;
            MX_VK_CHECK_RESULT(vkCreateImageView(manager->getDevice(), &viewInfo, nullptr, &imageView));
            return imageView;
        }

        VkDeviceMemory Image::allocateImageMemory(const Manager* manager, const VkImage image, const VkMemoryPropertyFlags properties) {
            VkMemoryRequirements memRequirements = {};
            vkGetImageMemoryRequirements(manager->getDevice(), image, &memRequirements);

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = manager->getMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

            VkDeviceMemory tempMemory;
            MX_VK_CHECK_RESULT(vkAllocateMemory(manager->getDevice(), &allocInfo, nullptr, &tempMemory));
            return tempMemory;
        }

        void Image::setImageLayout(VkCommandBuffer cmdbuffer,
                                   VkImage image,
                                   VkImageLayout oldImageLayout,
                                   VkImageLayout newImageLayout,
                                   VkImageSubresourceRange subresourceRange,
                                   VkPipelineStageFlags srcStageMask,
                                   VkPipelineStageFlags dstStageMask) {
            // Create an image barrier object
            VkImageMemoryBarrier imageMemoryBarrier = {};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.oldLayout = oldImageLayout; //旧的布局
            imageMemoryBarrier.newLayout = newImageLayout; //新的布局
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //当需要转移所有权(属于哪个队列簇)的时候
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //填入队列簇的索引
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.subresourceRange = subresourceRange;

            // Source layouts (old)
            // Source access mask controls actions that have to be finished on the old layout
            // before it will be transitioned to the new layout
            switch (oldImageLayout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined (or does not matter)
                // Only valid as initial layout
                // No flags required, listed only for completeness
                imageMemoryBarrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized
                // Only valid as initial layout for linear images, preserves memory contents
                // Make sure host writes have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment
                // Make sure any writes to the color buffer have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is a depth/stencil attachment
                // Make sure any writes to the depth/stencil buffer have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source 
                // Make sure any reads from the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination
                // Make sure any writes to the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is read by a shader
                // Make sure any shader reads from the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
            }

            // Target layouts (new)
            // Destination access mask controls the dependency for the new image layout
            switch (newImageLayout) {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as a transfer destination
                // Make sure any writes to the image have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as a transfer source
                // Make sure any reads from the image have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as a color attachment
                // Make sure any writes to the color buffer have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image layout will be used as a depth/stencil attachment
                // Make sure any writes to depth/stencil buffer have been finished
                imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be read in a shader (sampler, input attachment)
                // Make sure any writes to the image have been finished
                if (imageMemoryBarrier.srcAccessMask == 0) {
                    imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
            }

            // Put barrier inside setup command buffer
            vkCmdPipelineBarrier(
                cmdbuffer,
                srcStageMask,
                dstStageMask,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);
        }

        Image * Image::createDepthStencil(const Manager* manager, const VkFormat format, const VkExtent2D& extent, const VkSampleCountFlagBits sampleCount) {
            Image* image = new Image;

            image->image = Image::createImage2D(manager, extent, format,
                                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, sampleCount);
            image->memory = Image::allocateImageMemory(manager, image->image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            vkBindImageMemory(manager->getDevice(), image->image, image->memory, 0);
            image->view = Image::createImageView2D(manager, image->image, format, VK_IMAGE_ASPECT_DEPTH_BIT);

            image->extent = extent;
            image->format = format;

            return image;
        }
    }
}
