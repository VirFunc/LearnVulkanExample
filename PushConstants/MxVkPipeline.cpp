#include "MxVkPipeline.h"

namespace Mix {
    namespace Vk {
        Pipeline::Pipeline() :mPipeline(VK_NULL_HANDLE),
            mPipelineLayout(VK_NULL_HANDLE),
            mPipelineStates(nullptr) {
        }

        void Pipeline::clear() {
            if (mPipelineStates) {
                delete mPipelineStates;
                mPipelineStates = nullptr;
            }
        }

        bool Pipeline::setup(const Manager * manager) {
            if (mIsReady)
                destroy();

            mManager = manager;
            mPipelineStates = new PipelineStates;
            if (!mPipelineStates) {
                mManager = nullptr;
                mIsReady = false;
                return false;
            }

            mPipelineStates->vertexInput = {};
            mPipelineStates->vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            mPipelineStates->inputAssembly = {};
            mPipelineStates->inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

            mPipelineStates->rasterization = {};
            mPipelineStates->rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

            mPipelineStates->multisample = {};
            mPipelineStates->multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

            mPipelineStates->depthStencil = {};
            mPipelineStates->depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

            mPipelineStates->colorBlend = {};
            mPipelineStates->colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

            mIsReady = true;
            return true;
        }

        void Pipeline::setTargetRenderPass(const VkRenderPass renderPass, const uint32_t subpassIndex) {
            mRenderPass = renderPass;
            mSubpassIndex = subpassIndex;
        }

        void Pipeline::addShader(const VkShaderStageFlagBits stage, const VkShaderModule shader) {
            static char pName[] = "main";
            VkPipelineShaderStageCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            createInfo.stage = stage;
            createInfo.module = shader;
            createInfo.pName = pName;

            mPipelineStates->shaders.push_back(std::move(createInfo));
        }

        void Pipeline::setVertexInput(const std::vector<VkVertexInputBindingDescription>& bindingDescri, const std::vector<VkVertexInputAttributeDescription>& attriDescri) {
            mPipelineStates->vertexInput.pVertexBindingDescriptions = bindingDescri.data();
            mPipelineStates->vertexInput.vertexBindingDescriptionCount = bindingDescri.size();
            mPipelineStates->vertexInput.pVertexAttributeDescriptions = attriDescri.data();
            mPipelineStates->vertexInput.vertexAttributeDescriptionCount = attriDescri.size();
        }

        void Pipeline::setInputAssembly(const VkPrimitiveTopology topology, const bool primitiveRestart) {
            mPipelineStates->inputAssembly.topology = topology;
            mPipelineStates->inputAssembly.primitiveRestartEnable = primitiveRestart;
        }

        void Pipeline::addViewport(const std::vector<VkViewport>& viewports) {
            mPipelineStates->viewports.insert(mPipelineStates->viewports.end(), viewports.begin(), viewports.end());
        }

        void Pipeline::addViewport(const VkViewport& viewport) {
            mPipelineStates->viewports.push_back(viewport);
        }

        void Pipeline::addScissor(const std::vector<VkRect2D>& scissors) {
            mPipelineStates->scissors.insert(mPipelineStates->scissors.end(), scissors.begin(), scissors.end());
        }

        void Pipeline::addScissor(const VkRect2D & scissors) {
            mPipelineStates->scissors.push_back(scissors);
        }

        void Pipeline::setRasterization(const VkPolygonMode polygonMode, const VkCullModeFlags cullMode, const VkFrontFace frontFace, const float lineWidth, const bool depthClampEnable, const bool rasterizerDiscardEnable) {

            mPipelineStates->rasterization.polygonMode = polygonMode;
            mPipelineStates->rasterization.cullMode = cullMode;
            mPipelineStates->rasterization.frontFace = frontFace;
            mPipelineStates->rasterization.lineWidth = lineWidth;
            mPipelineStates->rasterization.depthClampEnable = depthClampEnable;
            mPipelineStates->rasterization.rasterizerDiscardEnable = rasterizerDiscardEnable;
        }

        void Pipeline::setDepthBias(const bool enable, const float constantFactor, const float slopeFactor, const float clamp) {
            mPipelineStates->rasterization.depthBiasEnable = enable;
            mPipelineStates->rasterization.depthBiasConstantFactor = constantFactor;
            mPipelineStates->rasterization.depthBiasSlopeFactor = slopeFactor;
            mPipelineStates->rasterization.depthBiasClamp = clamp;
        }

        void Pipeline::setMultiSample(const VkSampleCountFlagBits samples, const bool sampleShading, const float minSampleShading, const VkSampleMask * sampleMask, const bool alphaToCoverageEnable, const bool alphaToOneEnable) {

            mPipelineStates->multisample.sampleShadingEnable = sampleShading;
            mPipelineStates->multisample.rasterizationSamples = samples;
            mPipelineStates->multisample.minSampleShading = minSampleShading;
            mPipelineStates->multisample.pSampleMask = sampleMask;
            mPipelineStates->multisample.alphaToCoverageEnable = alphaToCoverageEnable;
            mPipelineStates->multisample.alphaToOneEnable = alphaToOneEnable;
        }

        void Pipeline::setDepthTest(const bool depthTestEnable, const bool depthWriteEnable, const VkCompareOp depthCompareOp) {
            mPipelineStates->depthStencil.depthTestEnable = depthTestEnable;
            mPipelineStates->depthStencil.depthWriteEnable = depthWriteEnable;
            mPipelineStates->depthStencil.depthCompareOp = depthCompareOp;
        }

        void Pipeline::setDepthBoundsTest(const bool enable, const float minBounds, const float maxBounds) {
            mPipelineStates->depthStencil.depthBoundsTestEnable = enable;
            mPipelineStates->depthStencil.minDepthBounds = minBounds;
            mPipelineStates->depthStencil.maxDepthBounds = maxBounds;
        }

        void Pipeline::setStencilTest(const bool enable, const VkStencilOpState & front, const VkStencilOpState & back) {
            mPipelineStates->depthStencil.stencilTestEnable = enable;
            mPipelineStates->depthStencil.front = front;
            mPipelineStates->depthStencil.back = back;
        }

        void Pipeline::addDefaultBlendAttachments() {
            VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
            colorBlendAttachment.blendEnable = true; //enable blend
            colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT |
                VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
            //finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            //finalColor.a   = (srcAlphaBlendFactor * newColor.a)   <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            addBlendAttachments(std::move(colorBlendAttachment));
        }

        void Pipeline::addBlendAttachments(const VkPipelineColorBlendAttachmentState & attachment) {
            mPipelineStates->colorBlendAttachments.push_back(attachment);
        }

        void Pipeline::addBlendAttachments(const std::vector<VkPipelineColorBlendAttachmentState>& attachments) {
            mPipelineStates->colorBlendAttachments.insert(mPipelineStates->colorBlendAttachments.end(), attachments.begin(), attachments.end());
        }

        void Pipeline::setBlend(const bool logicalOpEnable, const VkLogicOp logicOp, const float constantR, const float constantG, const float constantB, const float constantA) {
            mPipelineStates->colorBlend.logicOpEnable = logicalOpEnable;
            mPipelineStates->colorBlend.logicOp = logicOp;
            mPipelineStates->colorBlend.pAttachments = mPipelineStates->colorBlendAttachments.data();
            mPipelineStates->colorBlend.attachmentCount = mPipelineStates->colorBlendAttachments.size();
            mPipelineStates->colorBlend.blendConstants[0] = constantR;
            mPipelineStates->colorBlend.blendConstants[1] = constantG;
            mPipelineStates->colorBlend.blendConstants[2] = constantB;
            mPipelineStates->colorBlend.blendConstants[3] = constantA;
        }

        void Pipeline::addDynamicState(const VkDynamicState dynamicState) {
            mPipelineStates->dynamicStates.push_back(dynamicState);
        }

        void Pipeline::addDynamicState(const std::vector<VkDynamicState>& dynamicStates) {
            mPipelineStates->dynamicStates.insert(mPipelineStates->dynamicStates.end(), dynamicStates.begin(), dynamicStates.end());
        }

        void Pipeline::addDescriptorSetLayout(const VkDescriptorSetLayout setLayout) {
            mPipelineStates->descriptorSetLayouts.push_back(setLayout);
        }

        void Pipeline::addDescriptorSetLayout(const std::vector<VkDescriptorSetLayout>& setLayouts) {
            mPipelineStates->descriptorSetLayouts.insert(mPipelineStates->descriptorSetLayouts.end(), setLayouts.begin(), setLayouts.end());
        }

        void Pipeline::addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size) {
            mPipelineStates->pushConstantRanges.push_back({ stageFlags, offset, size });
        }

        void Pipeline::addPushConstantRange(const VkPushConstantRange & range) {
            mPipelineStates->pushConstantRanges.push_back(range);
        }

        void Pipeline::addPushConstantRanges(const std::vector<VkPushConstantRange>& ranges) {
            mPipelineStates->pushConstantRanges.insert(mPipelineStates->pushConstantRanges.end(), ranges.begin(), ranges.end());
        }

        bool Pipeline::createPipeline() {
            VkPipelineLayoutCreateInfo layoutCreateInfo = {};
            layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layoutCreateInfo.pSetLayouts = mPipelineStates->descriptorSetLayouts.data();
            layoutCreateInfo.setLayoutCount = mPipelineStates->descriptorSetLayouts.size();
            layoutCreateInfo.pPushConstantRanges = mPipelineStates->pushConstantRanges.data();
            layoutCreateInfo.pushConstantRangeCount = mPipelineStates->pushConstantRanges.size();

            if (vkCreatePipelineLayout(mManager->getDevice(), &layoutCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
                throw std::runtime_error("Error : Failed to create pipeline layout");

            VkPipelineViewportStateCreateInfo viewportState = {};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.pViewports = mPipelineStates->viewports.data();
            viewportState.viewportCount = mPipelineStates->viewports.size();
            viewportState.pScissors = mPipelineStates->scissors.data();
            viewportState.scissorCount = mPipelineStates->scissors.size();

            mPipelineStates->colorBlend.pAttachments = mPipelineStates->colorBlendAttachments.data();
            mPipelineStates->colorBlend.attachmentCount = mPipelineStates->colorBlendAttachments.size();

            VkPipelineDynamicStateCreateInfo dynamicState = {};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.pDynamicStates = mPipelineStates->dynamicStates.data();
            dynamicState.dynamicStateCount = mPipelineStates->dynamicStates.size();

            VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
            pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineCreateInfo.pStages = mPipelineStates->shaders.data();
            pipelineCreateInfo.stageCount = mPipelineStates->shaders.size();
            pipelineCreateInfo.pVertexInputState = &mPipelineStates->vertexInput;
            pipelineCreateInfo.pInputAssemblyState = &mPipelineStates->inputAssembly;
            pipelineCreateInfo.pViewportState = &viewportState;
            pipelineCreateInfo.pRasterizationState = &mPipelineStates->rasterization;
            pipelineCreateInfo.pMultisampleState = &mPipelineStates->multisample;
            pipelineCreateInfo.pDepthStencilState = &mPipelineStates->depthStencil;
            pipelineCreateInfo.pColorBlendState = &mPipelineStates->colorBlend;

            if (dynamicState.dynamicStateCount == 0)
                pipelineCreateInfo.pDynamicState = nullptr;
            else
                pipelineCreateInfo.pDynamicState = &dynamicState;

            pipelineCreateInfo.layout = mPipelineLayout;
            pipelineCreateInfo.renderPass = mRenderPass;
            pipelineCreateInfo.subpass = mSubpassIndex;
            pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;	//�������ߣ�vulkan�������Ѿ����ڵĹ����������µĹ��ߣ�
            pipelineCreateInfo.basePipelineIndex = -1;

            MX_VK_CHECK_RESULT(vkCreateGraphicsPipelines(mManager->getDevice(),
                               VK_NULL_HANDLE, 1,
                               &pipelineCreateInfo, nullptr,
                               &mPipeline));
            clear();
            return true;
        }

        void Pipeline::destroy() {
            if (!mIsReady)
                return;

            vkDestroyPipeline(mManager->getDevice(), mPipeline, nullptr);
            vkDestroyPipelineLayout(mManager->getDevice(), mPipelineLayout, nullptr);
            mManager = nullptr;
            mPipeline = VK_NULL_HANDLE;
            mPipelineLayout = VK_NULL_HANDLE;
        }

        Pipeline::~Pipeline() {
            destroy();
        }
    }
}