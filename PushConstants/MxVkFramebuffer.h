#pragma once
#ifndef _MX_VK_FRAMEBUFFER_H_
#define _MX_VK_FRAMEBUFFER_H_

#include"MxVkComponent.h"

#include<vector>

namespace Mix {
    namespace Vk {
        class Framebuffer :public Component {
        private:
            VkFramebuffer mFramebuffer;
            VkRenderPass mRenderPass;
            VkExtent2D mExtent;
            uint32_t mLayers;

            std::vector<VkImageView>* mAttachments;

            void clear();
        public:
            Framebuffer();
            bool setup(const Manager* manager);
            void setTargetRenderPass(const VkRenderPass renderPass) { mRenderPass = renderPass; };
            void setExtent(const VkExtent2D& extent) { mExtent = extent; };
            void setLayers(const uint32_t layer) { mLayers = layer; };
            void addAttachments(std::vector<VkImageView>& attachments);
            bool createFramebuffer();
            VkFramebuffer getFramebuffer() const { return mFramebuffer; };
            void destroy();
            ~Framebuffer();
        };
    }
}

#endif // !_MX_VK_FRAMEBUFFER_H_
