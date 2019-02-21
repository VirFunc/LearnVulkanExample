#include "Demo.h"

void TestDemo::run() {
    bool quit = false;
    SDL_Event event;
    while (!quit) {
        static auto start = std::chrono::system_clock::now();
        auto duration = std::chrono::system_clock::now() - start;
        float second = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() /
            float(std::chrono::microseconds::period::den);
        start = std::chrono::system_clock::now();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            }

            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_W:
                    mCamera.moveCamera(second, Mix::MxCamera::FORWARD);
                    break;
                case SDL_SCANCODE_S:
                    mCamera.moveCamera(second, Mix::MxCamera::BACK);
                    break;
                case SDL_SCANCODE_A:
                    mCamera.moveCamera(second, Mix::MxCamera::LEFT);
                    break;
                case SDL_SCANCODE_D:
                    mCamera.moveCamera(second, Mix::MxCamera::RIGHT);
                    break;
                case SDL_SCANCODE_SPACE:
                    mCamera.moveCamera(second, Mix::MxCamera::WORLD_UP);
                    break;
                case SDL_SCANCODE_LCTRL:
                    mCamera.moveCamera(second, Mix::MxCamera::WORLD_DOWN);
                    break;
                }
            }
        }
        draw();
    }
}

void TestDemo::draw() {
    vkWaitForFences(mManager->getDevice(), 1, &mInFlightFences[mCurrFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mManager->getDevice(), mSwapchain->getSwapchain(), std::numeric_limits<uint64_t>::max(),
                                            mImageAvailableSemaphores[mCurrFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        std::cerr << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
        return;
    } else
        MX_VK_CHECK_RESULT(result);

    updateUniformBuffer(imageIndex);

    //submit command buffer
    VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrFrame] };
    VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.pSignalSemaphores = signalSemaphores;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];
    submitInfo.commandBufferCount = 1;

    MX_VK_CHECK_RESULT(vkResetFences(mManager->getDevice(), 1, &mInFlightFences[mCurrFrame]));

    MX_VK_CHECK_RESULT(vkQueueSubmit(mManager->getQueue().graphics, 1, &submitInfo, mInFlightFences[mCurrFrame]));

    //swapchain present
    VkSwapchainKHR swapChains[] = { mSwapchain->getSwapchain() };

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.swapchainCount = 1;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(mManager->getQueue().present, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        std::cerr << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
        return;
    } else
        MX_VK_CHECK_RESULT(result);

    mCurrFrame = (mCurrFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void TestDemo::updateUniformBuffer(const uint32_t imageIndex) {
    UniformBufferObj ubo = {};
    ubo.model = glm::mat4(1.0f);

    mCamera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
    ubo.view = std::move(mCamera.getViewMatrix());

    ubo.proj = glm::perspective(glm::radians(45.0f),
                                mSwapchain->getCurrExtent().width / float(mSwapchain->getCurrExtent().height),
                                0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    mUniformBuffers[imageIndex]->map();
    mUniformBuffers[imageIndex]->copyTo(&ubo, sizeof(ubo));
    mUniformBuffers[imageIndex]->unmap();
}

TestDemo::TestDemo() {
    mWindow = new Mix::MxWindow;
    mManager = new Vk::Manager;
    mDebug = new Vk::Debug;
    mShaderManager = new Vk::ShaderManager;
    mSwapchain = new Vk::Swapchain;
    mRenderPass = new Vk::RenderPass;
    mDescriptorPool = new Vk::MxVkDescriptorPool;
    mDescriptorSetLayout = new Vk::DescriptorSetLayout;
    mPipeline = new Vk::Pipeline;
    mCommandPool = new Vk::CommandPool;
    mSyncObjectPool = new Vk::SyncObjectManager;

    mSampleCount = VK_SAMPLE_COUNT_1_BIT;
    mCurrFrame = 0;

    mModelImporter = new Vk::ModelImporter;
    if (!mModelImporter->initialize())
        throw std::runtime_error("Error : Failed to create model importer");
    mModel = mModelImporter->loadModel("Model\\robot.fbx");

    mCamera.setMoveSpeed(200.0f);
}

bool TestDemo::init() {
    mWindow->create("Test Demo", 600, 480);

    auto info = mManager->getEmptyInitInfo();

    info->debugMode = true;
    info->present = true;

    info->instance.validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");
    info->instance.appInfo.appName = mWindow->getTitle();
    info->instance.appInfo.appVersion = VK_MAKE_VERSION(0, 0, 1);
    info->instance.appInfo.engineName = "Mix";
    info->instance.appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);

    info->device.physical.type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    info->device.physical.enabledFeatures = { false };
    info->device.physical.extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    info->device.physical.queueFlags = VK_QUEUE_GRAPHICS_BIT;
    info->device.physical.queuePrioriy.graphics = 1.0f;

    info->window = mWindow->getWindow();

    try {
        mManager->initialize(*info);

        mDebug->setup(mManager);
        mShaderManager->setup(mManager);

        //setup debug
        mDebug->setDefaultCallback(Vk::Debug::SEVERITY_ALL, Vk::Debug::TYPE_ALL);

        //setup swapchain
        auto rect = mWindow->getWindowRect();
        mSwapchain->setup(mManager);
        mSwapchain->createSwapchain({ {VK_FORMAT_B8G8R8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } },
                                    VK_PRESENT_MODE_MAILBOX_KHR,
                                    { static_cast<uint32_t>(rect.width),static_cast<uint32_t>(rect.height) });

        //setup renderpass
        mRenderPass->setup(mManager);
        auto presentAttach = mRenderPass->addColorAttach(mSwapchain->getCurrFormat().format, mSampleCount,
                                                         VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
                                                         VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        auto presentAttachRef = mRenderPass->addColorAttachRef(presentAttach);

        auto depthAttach = mRenderPass->addDepthStencilAttach(VK_FORMAT_D24_UNORM_S8_UINT, mSampleCount);
        auto depthAttachRef = mRenderPass->addDepthStencilAttachRef(depthAttach);

        auto subpass = mRenderPass->addSubpass();
        mRenderPass->addSubpassColorRef(subpass, presentAttachRef);
        mRenderPass->addSubpassDepthStencilRef(subpass, depthAttachRef);

        mRenderPass->addDependency(VK_SUBPASS_EXTERNAL, subpass,
                                   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
        mRenderPass->createRenderPass();

        //setup descriptor
        mDescriptorSetLayout->setup(mManager);
        mDescriptorSetLayout->addBindings(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        mDescriptorSetLayout->createDescriptorSetLayout();

        //load shader
        mShaderManager->setup(mManager);
        auto vertexShader = mShaderManager->createModule("Shader/vShader.spv", VK_SHADER_STAGE_VERTEX_BIT);
        auto fragShader = mShaderManager->createModule("Shader/fShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

        //setup graphics pipeline
        mPipeline->setup(mManager);
        mPipeline->setTargetRenderPass(mRenderPass->getRenderPass(), 0);

        mPipeline->addShader(vertexShader->stage, vertexShader->module);
        mPipeline->addShader(fragShader->stage, fragShader->module);

        //setup vertex input state
        //todo create a new class to deal with this
        std::vector<VkVertexInputBindingDescription> inputBinding;
        std::vector<VkVertexInputAttributeDescription> inputAttri;
        inputBinding.push_back(Vk::Vertex::getBindingDescription());
        auto attri = Vk::Vertex::getAttributeDescription();
        inputAttri.insert(inputAttri.end(), attri.begin(), attri.end());
        mPipeline->setVertexInput(inputBinding, inputAttri);

        //setup input assembly
        mPipeline->setInputAssembly();

        //setup viewport
        mViewport.x = 0;
        mViewport.y = 0;
        mViewport.width = static_cast<float>(mWindow->getWindowRect().width);
        mViewport.height = static_cast<float>(mWindow->getWindowRect().height);
        mViewport.minDepth = 0.0f;
        mViewport.maxDepth = 1.0f;
        mPipeline->addViewport(mViewport);

        mScissor.extent = mSwapchain->getCurrExtent();
        mScissor.offset = { 0,0 };
        mPipeline->addScissor(mScissor);

        //setup rasterization
        mPipeline->setRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        mPipeline->setDepthBias(false);

        //setup multisample
        mPipeline->setMultiSample(mSampleCount);

        //setup depth/stencil test
        mPipeline->setDepthTest(true);
        mPipeline->setDepthBoundsTest(false);
        mPipeline->setStencilTest(false);

        //setup blend
        mPipeline->addDefaultBlendAttachments();

        //setup layout
        mPipeline->addDescriptorSetLayout(mDescriptorSetLayout->getDescriptorSetLayout());

        //add push constants
        mPipeline->addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT,
                                        0,
                                        sizeof(glm::vec4) * 3);

        //create graphics pipeline
        mPipeline->createPipeline();

        //create depth stencil buffer
        mDepthImage = Vk::Image::createDepthStencil(mManager, VK_FORMAT_D24_UNORM_S8_UINT,
                                                    mSwapchain->getCurrExtent(),
                                                    mSampleCount);

 //create command pool
        mCommandPool->setup(mManager);
        mCommandPool->createCommandPool(VK_QUEUE_GRAPHICS_BIT);

        //setup framebuffer
        std::vector<VkImageView> attachments;
        mFramebuffers.resize(mSwapchain->getImageCount());
        for (uint32_t i = 0; i < mFramebuffers.size(); ++i) {
            attachments = { mSwapchain->getImageViews()[i],mDepthImage->view };

            mFramebuffers[i] = new Vk::Framebuffer;
            mFramebuffers[i]->setup(mManager);
            mFramebuffers[i]->setExtent({ static_cast<uint32_t>(rect.width),static_cast<uint32_t>(rect.height) });
            mFramebuffers[i]->setLayers(1);
            mFramebuffers[i]->setTargetRenderPass(mRenderPass->getRenderPass());
            mFramebuffers[i]->addAttachments(attachments);
            mFramebuffers[i]->createFramebuffer();
        }

        mModel->createVertexBuffer(mManager, mCommandPool);

        //create uniform buffer
        {
            mUniformBuffers.resize(mSwapchain->getImageCount());
            for (size_t i = 0; i < mSwapchain->getImageCount(); ++i) {
                mUniformBuffers[i] = Vk::Buffer::createBuffer(mManager, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                              sizeof(UniformBufferObj));
            }
        }

        //create descriptor pool
        mDescriptorPool->setup(mManager);
        mDescriptorPool->addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, mSwapchain->getImageCount());
        mDescriptorPool->createDescriptorPool(mSwapchain->getImageCount());

        //allocate descriptor sets
        mDescriptorSets = mDescriptorPool->allocDescriptorSet(*mDescriptorSetLayout, mSwapchain->getImageCount());

        //update descriptor sets
        for (size_t i = 0; i < mSwapchain->getImageCount(); ++i) {
            std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = mDescriptorSets[i]; //descriptor which will be write in
            descriptorWrites[0].dstBinding = 0; //destination binding
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //the type of the descriptor that will be wirte in
            descriptorWrites[0].descriptorCount = 1; //descriptor count
            descriptorWrites[0].pBufferInfo = &mUniformBuffers[i]->descriptor; //descriptorBufferInfo
            descriptorWrites[0].pImageInfo = nullptr;
            descriptorWrites[0].pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(mManager->getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
        }

        //create commandbuffer
        {
            mCommandBuffers = mCommandPool->allocCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, mSwapchain->getImageCount());

            for (size_t i = 0; i < mCommandBuffers.size(); ++i) {
                //begin command buffer
                Vk::beginCommandBuffer(mCommandBuffers[i]);

                std::vector<VkClearValue> clearValues(2);
                clearValues[0].color = { 0.0f,0.0f,0.0f,1.0f };
                clearValues[1].depthStencil = { 1.0f,0 };

                //begin render pass
                mRenderPass->beginRenderPass(mCommandBuffers[i], mFramebuffers[i]->getFramebuffer(), clearValues,
                                             mSwapchain->getCurrExtent());

                // push constants
                //bind pipeline
                vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->getPipeline());

                std::cout << "max push constants size : "
                    << mManager->getPhysicalDeviceProperties().limits.maxPushConstantsSize
                    << std::endl;

                if (sizeof(glm::vec3) * 3 > mManager->getPhysicalDeviceProperties().limits.maxPushConstantsSize) {
                    std::cerr << "Error : Push constants size out." << std::endl;
                } else {
                    mPushConstants[0] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
                    mPushConstants[1] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
                    mPushConstants[2] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

                    // in glsl
                    // vec3 isn't actually a 3-component vector, 
                    // it's just an alias to a vec4 with proper type checks 

                    /*vec3 1.0f, 0.0f, 0.0f             vec4 1.0f, 0.0f, 0.0f, 0.0f
                      vec3 0.0f, 1.0f, 0.0f      ==>    vec4 1.0f, 0.0f, 0.0f, 0.0f
                      vec3 0.0f, 0.0f, 1.0f             vec4 1.0f, ----------------*/

                    vkCmdPushConstants(mCommandBuffers[i],
                                       mPipeline->getPipelineLayout(),
                                       VK_SHADER_STAGE_VERTEX_BIT,
                                       0,
                                       sizeof(glm::vec4) * 3,
                                       mPushConstants);
                }

                //bind descriptor sets
                vkCmdBindDescriptorSets(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->getPipelineLayout(),
                                        0, // the index of the first descriptor set
                                        1, // descriptor count
                                        &mDescriptorSets[i], // descriptor array
                                        0, nullptr);

                //draw
                mModel->draw(mCommandBuffers[i]);

                //end render pass
                mRenderPass->endRenderPass(mCommandBuffers[i]);

                //end command buffer
                Vk::endCommandBuffer(mCommandBuffers[i]);
            }
        }

        //create fence and semaphore
        mSyncObjectPool->setup(mManager);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            mImageAvailableSemaphores.push_back(mSyncObjectPool->createSemaphore());
            mRenderFinishedSemaphores.push_back(mSyncObjectPool->createSemaphore());
            mInFlightFences.push_back(mSyncObjectPool->createFence());
        }
    }

    catch (const std::exception& e) {
        std::cout << e.what() << std::ends << std::endl;
    }

    return true;
}

void TestDemo::destroy() {
    vkDeviceWaitIdle(mManager->getDevice());

    MX_FREE_OBJECT(mSyncObjectPool);
    MX_FREE_OBJECT(mCommandPool);

    vkDestroyImageView(mManager->getDevice(), mDepthImage->view, nullptr);
    vkDestroyImage(mManager->getDevice(), mDepthImage->image, nullptr);
    vkFreeMemory(mManager->getDevice(), mDepthImage->memory, nullptr);

    for (auto& framebuffer : mFramebuffers)
        MX_FREE_OBJECT(framebuffer);

    MX_FREE_OBJECT(mShaderManager);
    MX_FREE_OBJECT(mSwapchain);
    MX_FREE_OBJECT(mDescriptorSetLayout);
    MX_FREE_OBJECT(mDescriptorPool);
    MX_FREE_OBJECT(mPipeline);
    MX_FREE_OBJECT(mRenderPass);

    MX_FREE_OBJECT(mModel);
    for (auto& buffer : mUniformBuffers)
        MX_FREE_OBJECT(buffer);

    MX_FREE_OBJECT(mDebug);
    MX_FREE_OBJECT(mManager);
    MX_FREE_OBJECT(mWindow);
}
