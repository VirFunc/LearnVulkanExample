#include "MxVkSwapchain.h"

namespace Mix {
    namespace Vk {
        Swapchain::Swapchain() {
        }

        Swapchain::SwapchainSupportDetails Swapchain::getSwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
            {//��ȡ������ϸ����Ϣ
                SwapchainSupportDetails details;
                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

                uint32_t formatCount;
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
                if (formatCount != 0) {
                    details.formats.resize(formatCount);
                    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
                }

                uint32_t presentModeCount;
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
                if (presentModeCount != 0) {
                    details.presentModes.resize(presentModeCount);
                    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
                }

                return details;
            }
        }

        bool Swapchain::chooseFormat(const std::vector<VkSurfaceFormatKHR>& rqFormats, VkSurfaceFormatKHR& format) {
            if (mSwapchainSupportDetails.formats.size() == 1 && mSwapchainSupportDetails.formats[0].format == VK_FORMAT_UNDEFINED) {
                format = rqFormats[0];
                return true;
            }

            for (const auto& rqFormat : rqFormats) {
                for (const auto& support : mSwapchainSupportDetails.formats) {
                    if (rqFormat.format == support.format && rqFormat.colorSpace == support.colorSpace) {
                        format = support;
                        return true;
                    }
                }
            }
            return false;
        }

        bool Swapchain::choosePresentMode(const VkPresentModeKHR rqPresentMode, VkPresentModeKHR & presentMode) {
            for (const auto& support : mSwapchainSupportDetails.presentModes) {
                if (rqPresentMode == support) {
                    presentMode = support;
                    return true;
                }
            }
            presentMode = VK_PRESENT_MODE_FIFO_KHR;
            return false;
        }

        void Swapchain::createSwapchainImageView() {
            mSwapChainImageViews.resize(mSwapChainImages.size());
            for (uint32_t i = 0; i < mSwapChainImages.size(); ++i) {
                mSwapChainImageViews[i] = Image::createImageView2D(mManager,
                                                                   mSwapChainImages[i],
                                                                   mCurrFormat.format,
                                                                   VK_IMAGE_ASPECT_COLOR_BIT,
                                                                   0, 1, 0, 1);
            }
        }

        VkExtent2D Swapchain::chooseExtent(const VkExtent2D & rqExtent) {
            if (mSwapchainSupportDetails.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return mSwapchainSupportDetails.capabilities.currentExtent;
            } else {
                VkExtent2D actualExtent = { static_cast<uint32_t>(rqExtent.width), static_cast<uint32_t>(rqExtent.height) };
                actualExtent.width = std::max(mSwapchainSupportDetails.capabilities.minImageExtent.width,
                                              std::min(mSwapchainSupportDetails.capabilities.maxImageExtent.width, actualExtent.width));
                actualExtent.height = std::max(mSwapchainSupportDetails.capabilities.minImageExtent.height,
                                               std::min(mSwapchainSupportDetails.capabilities.maxImageExtent.height, actualExtent.height));
                return actualExtent;
            }
        }

        bool Swapchain::setup(const Manager* manager) {
            if (mIsReady)
                destroy();
            mManager = manager;
            mIsReady = true;
            mSwapchainSupportDetails = getSwapChainSupport(mManager->getPhysicalDevice(), mManager->getSurface());
            return true;
        }

        std::vector<VkSurfaceFormatKHR> Swapchain::getSupportFormat() const {
            if (mIsReady)
                return mSwapchainSupportDetails.formats;
            else
                return std::vector<VkSurfaceFormatKHR>();
        }

        std::vector<VkPresentModeKHR> Swapchain::getSupportPresentMode() const {
            if (mIsReady)
                return mSwapchainSupportDetails.presentModes;
            else
                return std::vector<VkPresentModeKHR>();
        }

        bool Swapchain::createSwapchain(const std::vector<VkSurfaceFormatKHR>& rqFormats, VkPresentModeKHR rqPresentMode, VkExtent2D rqExtent) {
            if (!mIsReady)
                return false;

            //������surface��ʽ
            VkSurfaceFormatKHR format;
            if (!chooseFormat(rqFormats, format))
                return false;

            //����������ģʽ
            VkPresentModeKHR presentMode;
            if (!choosePresentMode(rqPresentMode, presentMode)) {
                return false;
            }
            //������
            VkExtent2D extent = chooseExtent(rqExtent);

            //��������image������
            uint32_t imageCount = mSwapchainSupportDetails.capabilities.minImageCount;
            if (imageCount > 0 && imageCount > mSwapchainSupportDetails.capabilities.maxImageCount)
                imageCount = mSwapchainSupportDetails.capabilities.maxImageCount;

            //����������
            VkSwapchainCreateInfoKHR createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = mManager->getSurface();
            createInfo.presentMode = presentMode;
            createInfo.minImageCount = imageCount;
            //��ʽ
            createInfo.imageFormat = format.format;
            //ɫ�ʿռ�
            createInfo.imageColorSpace = format.colorSpace;
            //ͼ���С
            createInfo.imageExtent = extent;
            //ÿһ��image�����Ĳ���
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //image����;

            uint32_t queueFamilyIndices[] = { mManager->getQueueFamilyIndices().graphics,
                                              mManager->getQueueFamilyIndices().present };
            if (queueFamilyIndices[0] != queueFamilyIndices[1]) {//���graphics����present�ز�ͬ
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; //����
                //�����ͬ��ͨ��queueFamilyIndices�����ֲ�ͬ�Ķ��д�
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
                createInfo.queueFamilyIndexCount = 2;
            } else {//���graphics����present����ͬ
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //��ռ
                //��ռ����Ҫ����һ����Ϣ
                createInfo.queueFamilyIndexCount = 0;
                createInfo.pQueueFamilyIndices = nullptr;
            }

            //�Խ������е�image������Ԥ�任
            createInfo.preTransform = mSwapchainSupportDetails.capabilities.currentTransform;
            //ȷ�����봰��ϵͳ�е��������ڻ�����Ƿ�ʹ��alphaͨ��
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            //�����ڱ����������ڵ�ʱ���Ƿ�ü������ڵ��Ĳ���
            createInfo.clipped = true;
            //�������������ı�ʱ��������һ��������
            createInfo.oldSwapchain = VK_NULL_HANDLE;

            //����������
            MX_VK_CHECK_RESULT(vkCreateSwapchainKHR(mManager->getDevice(), &createInfo, nullptr, &mSwapchain));
            //��ȡ�������е�image
            vkGetSwapchainImagesKHR(mManager->getDevice(), mSwapchain, &imageCount, nullptr);
            mSwapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(mManager->getDevice(), mSwapchain, &imageCount, mSwapChainImages.data());
            //���潻�����ĸ�ʽ�ʹ�С
            mCurrFormat = format;
            mCurrExtent = extent;
            mCurrPresentMode = presentMode;
            createSwapchainImageView();
            return true;
        }

        void Swapchain::destroy() {
            if (!mIsReady)
                return;
            if (mSwapchain != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(mManager->getDevice(), mSwapchain, nullptr);
                mSwapchain = VK_NULL_HANDLE;
            }
            for (const auto& view : mSwapChainImageViews) {
                vkDestroyImageView(mManager->getDevice(), view, nullptr);
            }
            mSwapChainImages.clear();
            mSwapChainImageViews.clear();
            mSwapchainSupportDetails.formats.clear();
            mSwapchainSupportDetails.presentModes.clear();
            mManager = nullptr;
        }

        Swapchain::~Swapchain() {
            destroy();
        }
    }
}