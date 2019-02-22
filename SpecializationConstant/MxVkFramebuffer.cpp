#include "MxVkFramebuffer.h"

namespace Mix {
    namespace Vk {
        void Framebuffer::clear() {
            if (mAttachments)
                delete mAttachments;
        }
        Framebuffer::Framebuffer() :mRenderPass(VK_NULL_HANDLE) {
        }

        bool Framebuffer::setup(const Manager * manager) {
            if (mIsReady)
                destroy();

            mAttachments = new std::vector<VkImageView>;
            if (!mAttachments)
                return false;
            mManager = manager;
            mIsReady = true;
            return true;
        }

        void Framebuffer::addAttachments(std::vector<VkImageView>& attachments) {
            mAttachments->insert(mAttachments->end(), attachments.begin(), attachments.end());
        }

        bool Framebuffer::createFramebuffer() {
            VkFramebufferCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.renderPass = mRenderPass;
            createInfo.pAttachments = mAttachments->data();
            createInfo.attachmentCount = mAttachments->size();
            createInfo.width = mExtent.width;
            createInfo.height = mExtent.height;
            createInfo.layers = mLayers;

            MX_VK_CHECK_RESULT(vkCreateFramebuffer(mManager->getDevice(), &createInfo, nullptr, &mFramebuffer));
            return true;
        }

        void Framebuffer::destroy() {
            if (!mIsReady)
                return;

            if (mAttachments)
                delete mAttachments;
            vkDestroyFramebuffer(mManager->getDevice(), mFramebuffer, nullptr);
            mAttachments = nullptr;
            mManager = nullptr;
            mRenderPass = VK_NULL_HANDLE;
            mFramebuffer = VK_NULL_HANDLE;
            mExtent = { 0,0 };
            mLayers = 0;
        }

        Framebuffer::~Framebuffer() {
            destroy();
        }
    }
}
