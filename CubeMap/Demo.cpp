#include "Demo.h"

//debug test----------------------------------
void drawSlipLine() {
    std::cout << "------------------------" << std::endl;
}

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec3) {
    return os << "[ " << vec3.x << ", " << vec3.y << ", " << vec3.z << " ]";
}

std::ostream& operator<<(std::ostream& os, const Mix::Axis& axis) {
    return os << "{ " << axis.x << ", " << axis.y << ", " << axis.z << " }";
}

std::ostream& operator<<(std::ostream& os, const glm::quat& quat) {
    return os << "[ " << quat.x << ", " << quat.y << ", " << quat.z << ", " << quat.w << " ]";
}

std::ostream& operator<<(std::ostream& os, const Mix::Transform& trans) {
    std::cout << trans.getPosition() << std::endl;
    std::cout << trans.getAxis() << std::endl;
    return std::cout << trans.getEulerAngles() << std::endl;
}
//debug test----------------------------------

void TestDemo::run() {
    setupRs();
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        static auto last = std::chrono::system_clock::now();
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - last);
        float seconds = float(duration.count())*decltype(duration)::period::num / decltype(duration)::period::den;

        if (seconds > FIXED_UPDATE_TIMESTEP) {
            last = now;

            keyboardStateEvent(seconds);
            updateScene(seconds);

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    quit = true;
                    break;
                }

                switch (event.type) {
                case SDL_MOUSEMOTION:
                    mouseMoveEvent(event, seconds);
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        quit = true;
                        break;
                    }
                    break;
                }
            }
        }
        draw();
    }
}

void TestDemo::draw() {
    vkWaitForFences(mManager->getDevice(),
                    1,
                    &mInFlightFences[mCurrFrame],
                    VK_TRUE,
                    std::numeric_limits<uint64_t>::max());

    MX_VK_CHECK_RESULT(vkResetFences(mManager->getDevice(), 1, &mInFlightFences[mCurrFrame]));

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mManager->getDevice(),
                                            mSwapchain->getSwapchain(),
                                            std::numeric_limits<uint64_t>::max(),
                                            mImageAvailableSemaphores[mCurrFrame],
                                            VK_NULL_HANDLE,
                                            &imageIndex);

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

void TestDemo::loadResources() {
    mModelImporter = new Vk::ModelImporter;
    if (!mModelImporter->initialize())
        throw std::runtime_error("Error : Failed to create model importer");

    mSkyboxModel = mModelImporter->loadModel("Model\\cube.fbx");
    mObjectModel = mModelImporter->loadModel("Model\\robot.fbx");
}

void TestDemo::setupRs() {
    mCamera.setMoveSpeed(1.0f);
    mCamera.setPerspective(45.0f, float(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 100.0f);
    mCamera.setPosition(0.0f, 0.0f, 0.0f);
    mCamera.setMouseSensitivity(1.5f);
}

void TestDemo::updateScene(const float deltaTime) {

}

void TestDemo::keyboardStateEvent(const float deltaTime) {
    if (mKeyboardState[SDL_SCANCODE_W])
        mCamera.moveCamera(deltaTime, Mix::Camera::FORWARD);
    if (mKeyboardState[SDL_SCANCODE_S])
        mCamera.moveCamera(deltaTime, Mix::Camera::BACK);
    if (mKeyboardState[SDL_SCANCODE_A])
        mCamera.moveCamera(deltaTime, Mix::Camera::LEFT);
    if (mKeyboardState[SDL_SCANCODE_D])
        mCamera.moveCamera(deltaTime, Mix::Camera::RIGHT);
    if (mKeyboardState[SDL_SCANCODE_SPACE])
        mCamera.moveCamera(deltaTime, Mix::Camera::WORLD_UP);
    if (mKeyboardState[SDL_SCANCODE_LCTRL])
        mCamera.moveCamera(deltaTime, Mix::Camera::WORLD_DOWN);
}

void TestDemo::mouseMoveEvent(SDL_Event & e, const float deltaTime) {
    mCamera.rotateCamera(-glm::radians(float(e.motion.xrel)),
                         glm::radians(float(e.motion.yrel)),
                         0.0f,
                         deltaTime);
}

void TestDemo::updateUniformBuffer(const uint32_t imageIndex) {
    UniformBufferObj ubo = {};
    ubo.view = std::move(mCamera.getViewMatrix());
    ubo.proj = mCamera.getProjMat();
    ubo.proj[1][1] *= -1;
    ubo.viewPos = glm::vec4(mCamera.getTransform().getPosition(), 1.0f);

    ubo.model = glm::translate(glm::mat4(1.0f), mCamera.getPosition());

    mSkyboxUniform[imageIndex]->map();
    mSkyboxUniform[imageIndex]->copyTo(&ubo, sizeof(ubo));
    mSkyboxUniform[imageIndex]->unmap();

    ubo.model = glm::mat4(1.0f);
    ubo.normal = glm::transpose(glm::inverse(ubo.model));

    mObjectUniform[imageIndex]->map();
    mObjectUniform[imageIndex]->copyTo(&ubo, sizeof(ubo));
    mObjectUniform[imageIndex]->unmap();
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
    mPipelineManager = new Vk::PipelineManager;
    mCommandPool = new Vk::CommandPool;
    mSyncObjectPool = new Vk::SyncObjectManager;

    mSampleCount = VK_SAMPLE_COUNT_1_BIT;
    mCurrFrame = 0;
}

void TestDemo::initWindow() {
    mWindow->create("Test Demo", WINDOW_WIDTH, WINDOW_HEIGHT);
    mKeyboardState = SDL_GetKeyboardState(nullptr);
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void TestDemo::initManager() {
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

    mManager->initialize(*info);
    delete info;
}

void TestDemo::initDebugUtils() {
    mDebug->setup(mManager);
    mDebug->setDefaultCallback(Vk::Debug::SEVERITY_ALL, Vk::Debug::TYPE_ALL);
}

void TestDemo::initSwapchain() {
    auto rect = mWindow->getWindowRect();
    mSwapchain->setup(mManager);
    mSwapchain->createSwapchain({ {VK_FORMAT_B8G8R8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } },
                                VK_PRESENT_MODE_FIFO_KHR,
                                { static_cast<uint32_t>(rect.width),static_cast<uint32_t>(rect.height) });
}

void TestDemo::initRenderPass() {
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
}

void TestDemo::initDescriptorSetLayout() {
    mDescriptorSetLayout->setup(mManager);
    mDescriptorSetLayout->addBindings(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    mDescriptorSetLayout->addBindings(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
    mDescriptorSetLayout->createDescriptorSetLayout();
}

void TestDemo::initShaders() {
    //load shader
    mShaderManager->setup(mManager);
    mShaderManager->createModule("vSkybox", "Shader/vSkybox.spv", VK_SHADER_STAGE_VERTEX_BIT);
    mShaderManager->createModule("fSkybox", "Shader/fSkybox.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    mShaderManager->createModule("vObject", "Shader/vObject.spv", VK_SHADER_STAGE_VERTEX_BIT);
    mShaderManager->createModule("fObject", "Shader/fObject.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
}

void TestDemo::initPipeline() {
    mPipelineManager->setup(mManager);

    std::vector<VkVertexInputBindingDescription> inputBinding;
    std::vector<VkVertexInputAttributeDescription> inputAttri;
    inputBinding.push_back(Vk::Vertex::getBindingDescription());
    auto attri = Vk::Vertex::getAttributeDescription();
    inputAttri.insert(inputAttri.end(), attri.begin(), attri.end());

    //prepare skybox pipeline
    {
        auto& pipeline = mPipelineManager->createPipeline("skybox", mRenderPass->getRenderPass(), 0);

        //setup graphics pipeline
        auto& vertexShader = mShaderManager->getModule("vSkybox");
        auto& fragShader = mShaderManager->getModule("fSkybox");

        pipeline.addShader(vertexShader.stage, vertexShader.module);
        pipeline.addShader(fragShader.stage, fragShader.module);

         //setup vertex input state
         //todo create a new class to deal with this
        pipeline.setVertexInput(inputBinding, inputAttri);

         //setup input assembly
        pipeline.setInputAssembly();

         //setup viewport
        mViewport.x = 0;
        mViewport.y = 0;
        mViewport.width = static_cast<float>(mWindow->getWindowRect().width);
        mViewport.height = static_cast<float>(mWindow->getWindowRect().height);
        mViewport.minDepth = 0.0f;
        mViewport.maxDepth = 1.0f;
        pipeline.addViewport(mViewport);

        mScissor.extent = mSwapchain->getCurrExtent();
        mScissor.offset = { 0,0 };
        pipeline.addScissor(mScissor);

         //setup rasterization
        pipeline.setRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        pipeline.setDepthBias(false);

         //setup multisample
        pipeline.setMultiSample(mSampleCount);

         //setup depth/stencil test
        pipeline.setDepthTest(false);
        pipeline.setDepthBoundsTest(false);
        pipeline.setStencilTest(false);

         //setup blend
        pipeline.addDefaultBlendAttachments();

         //setup layout
        pipeline.addDescriptorSetLayout(mDescriptorSetLayout->getDescriptorSetLayout());

         //create graphics pipeline
        pipeline.createPipeline();
    }

    //prepare object pipeline
    {
        auto& pipeline = mPipelineManager->createPipeline("object", mRenderPass->getRenderPass(), 0);

           //setup graphics pipeline
        auto& vertexShader = mShaderManager->getModule("vObject");
        auto& fragShader = mShaderManager->getModule("fObject");

        pipeline.addShader(vertexShader.stage, vertexShader.module);
        pipeline.addShader(fragShader.stage, fragShader.module);

         //setup vertex input state
         //todo create a new class to deal with this
        pipeline.setVertexInput(inputBinding, inputAttri);

         //setup input assembly
        pipeline.setInputAssembly();

         //setup viewport
        pipeline.addViewport(mViewport);

        pipeline.addScissor(mScissor);

         //setup rasterization
        pipeline.setRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        pipeline.setDepthBias(false);

         //setup multisample
        pipeline.setMultiSample(mSampleCount);

         //setup depth/stencil test
        pipeline.setDepthTest(true);
        pipeline.setDepthBoundsTest(false);
        pipeline.setStencilTest(false);

         //setup blend
        pipeline.addDefaultBlendAttachments();

         //setup layout
        pipeline.addDescriptorSetLayout(mDescriptorSetLayout->getDescriptorSetLayout());

         //create graphics pipeline
        pipeline.createPipeline();
    }
}

void TestDemo::initDepthStencilImage() {
    //create depth stencil buffer
    mDepthImage = Vk::Image::createDepthStencil(mManager, VK_FORMAT_D24_UNORM_S8_UINT,
                                                mSwapchain->getCurrExtent(),
                                                mSampleCount);
}

void TestDemo::initCommandPool() {
    mCommandPool->setup(mManager);
    mCommandPool->createCommandPool(VK_QUEUE_GRAPHICS_BIT);
}

void TestDemo::initFrameBuffers() {
    std::vector<VkImageView> attachments;
    mFramebuffers.resize(mSwapchain->getImageCount());

    for (uint32_t i = 0; i < mFramebuffers.size(); ++i) {
        attachments = { mSwapchain->getImageViews()[i],mDepthImage->view };

        mFramebuffers[i] = new Vk::Framebuffer;
        mFramebuffers[i]->setup(mManager);
        mFramebuffers[i]->setExtent({ static_cast<uint32_t>(mWindow->getWindowRect().width),
                                    static_cast<uint32_t>(mWindow->getWindowRect().height) });
        mFramebuffers[i]->setLayers(1);
        mFramebuffers[i]->setTargetRenderPass(mRenderPass->getRenderPass());
        mFramebuffers[i]->addAttachments(attachments);
        mFramebuffers[i]->createFramebuffer();
    }
}

void TestDemo::initVertexBuffers() {
    mSkyboxModel->createVertexBuffer(mManager, mCommandPool);
    mObjectModel->createVertexBuffer(mManager, mCommandPool);
}

void TestDemo::initUniformBuffers() {
    mSkyboxUniform.resize(mSwapchain->getImageCount());
    for (size_t i = 0; i < mSwapchain->getImageCount(); ++i) {
        mSkyboxUniform[i] = Vk::Buffer::createBuffer(mManager, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                     sizeof(UniformBufferObj));
    }

    mObjectUniform.resize(mSwapchain->getImageCount());
    for (size_t i = 0; i < mSwapchain->getImageCount(); ++i) {
        mObjectUniform[i] = Vk::Buffer::createBuffer(mManager, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                     sizeof(UniformBufferObj));
    }
}

void TestDemo::initTexture() {
    int texWidth, texHeight, texChannels;

    stbi_set_flip_vertically_on_load(true);
    stbi_uc* cubeMaps[6];
    cubeMaps[0] = stbi_load("Texture/front.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    cubeMaps[1] = stbi_load("Texture/back.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    cubeMaps[2] = stbi_load("Texture/left.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    cubeMaps[3] = stbi_load("Texture/right.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    cubeMaps[4] = stbi_load("Texture/top.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    cubeMaps[5] = stbi_load("Texture/bottom.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);


    VkDeviceSize imageSize = texWidth * texHeight * 4 * 6;
    VkDeviceSize layerSize = imageSize / 6;

    Vk::Buffer* stagingBuffer = Vk::Buffer::createBuffer(mManager,
                                                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                         imageSize,
                                                         VK_SHARING_MODE_EXCLUSIVE);
    {
        VkDeviceSize offset = 0;
        for (int i = 0; i < 6; ++i) {
            stagingBuffer->map(imageSize - offset, offset);
            stagingBuffer->copyTo(cubeMaps[i], layerSize);
            stagingBuffer->unmap();
            offset += layerSize;
        }
    }

    for (int i = 0; i < 6; ++i) {
        stbi_image_free(cubeMaps[i]);
    }

    mCubeImage.extent = {
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight)
    };
    mCubeImage.format = VK_FORMAT_R8G8B8A8_UNORM;

    VkImageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.extent.width = mCubeImage.extent.width;
    createInfo.extent.height = mCubeImage.extent.height;
    createInfo.extent.depth = 1;
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 6;
    createInfo.format = mCubeImage.format;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    MX_VK_CHECK_RESULT(vkCreateImage(mManager->getDevice(), &createInfo, nullptr, &mCubeImage.image));

    mCubeImage.memory = Vk::Image::allocateImageMemory(mManager,
                                                       mCubeImage.image,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    MX_VK_CHECK_RESULT(vkBindImageMemory(mManager->getDevice(), mCubeImage.image, mCubeImage.memory, 0));

    auto copyCmd = mCommandPool->beginTempCommandBuffer();

    std::vector<VkBufferImageCopy> bufferCopyRegions;
    VkDeviceSize offset = 0;

    for (uint32_t face = 0; face < 6; ++face) {
        VkBufferImageCopy bufferCopyRegion = {};
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = 0;
        bufferCopyRegion.imageSubresource.baseArrayLayer = face;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = mCubeImage.extent.width;
        bufferCopyRegion.imageExtent.height = mCubeImage.extent.height;
        bufferCopyRegion.imageExtent.depth = 1;
        bufferCopyRegion.bufferOffset = offset;

        bufferCopyRegions.push_back(bufferCopyRegion);

        offset += layerSize;
    }

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 6;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;

    Vk::Image::setImageLayout(copyCmd, mCubeImage.image,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              subresourceRange,
                              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                              VK_PIPELINE_STAGE_TRANSFER_BIT);

    vkCmdCopyBufferToImage(copyCmd,
                           stagingBuffer->buffer, mCubeImage.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(bufferCopyRegions.size()),
                           bufferCopyRegions.data());

    Vk::Image::setImageLayout(copyCmd, mCubeImage.image,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              subresourceRange,
                              VK_PIPELINE_STAGE_TRANSFER_BIT,
                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    mCommandPool->endTempCommandBuffer(copyCmd);

    VkSamplerCreateInfo sampler = {};
    sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler.minFilter = VK_FILTER_LINEAR;
    sampler.magFilter = VK_FILTER_LINEAR;
    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler.unnormalizedCoordinates = VK_FALSE;
    sampler.compareEnable = VK_FALSE;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.mipLodBias = 0.0f;
    sampler.minLod = 0.0f;
    sampler.maxLod = 0.0f;
    sampler.anisotropyEnable = VK_FALSE;

    MX_VK_CHECK_RESULT(vkCreateSampler(mManager->getDevice(), &sampler, nullptr, &mCubeSampler));

    VkImageViewCreateInfo view = {};
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    view.format = mCubeImage.format;
    view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };;
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.levelCount = 1;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 6;
    view.image = mCubeImage.image;

    MX_VK_CHECK_RESULT(vkCreateImageView(mManager->getDevice(), &view, nullptr, &mCubeImage.view));

    delete stagingBuffer;
}

void TestDemo::initDescriptorSets() {
    //create descriptor pool
    mDescriptorPool->setup(mManager);
    mDescriptorPool->addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, mSwapchain->getImageCount() * 2);
    mDescriptorPool->addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, mSwapchain->getImageCount() * 2);
    mDescriptorPool->createDescriptorPool(mSwapchain->getImageCount() * 2);

    //allocate descriptor sets
    mSkyboxDescriptor = mDescriptorPool->allocDescriptorSet(*mDescriptorSetLayout, mSwapchain->getImageCount());
    mObjectDescriptor = mDescriptorPool->allocDescriptorSet(*mDescriptorSetLayout, mSwapchain->getImageCount());

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = mCubeImage.view;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.sampler = mCubeSampler;

    //update descriptor sets
    for (size_t i = 0; i < mSwapchain->getImageCount(); ++i) {
        std::array<VkWriteDescriptorSet, 4> descriptorWrites = {};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = mSkyboxDescriptor[i]; //descriptor which will be write in
        descriptorWrites[0].dstBinding = 0; //destination binding
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //the type of the descriptor that will be wirte in
        descriptorWrites[0].descriptorCount = 1; //descriptor count
        descriptorWrites[0].pBufferInfo = &mSkyboxUniform[i]->descriptor; //descriptorBufferInfo
        descriptorWrites[0].pImageInfo = nullptr;
        descriptorWrites[0].pTexelBufferView = nullptr;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = mSkyboxDescriptor[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = nullptr;
        descriptorWrites[1].pImageInfo = &imageInfo;
        descriptorWrites[1].pTexelBufferView = nullptr;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = mObjectDescriptor[i]; //descriptor which will be write in
        descriptorWrites[2].dstBinding = 0; //destination binding
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //the type of the descriptor that will be wirte in
        descriptorWrites[2].descriptorCount = 1; //descriptor count
        descriptorWrites[2].pBufferInfo = &mObjectUniform[i]->descriptor; //descriptorBufferInfo
        descriptorWrites[2].pImageInfo = nullptr;
        descriptorWrites[2].pTexelBufferView = nullptr;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = mObjectDescriptor[i];
        descriptorWrites[3].dstBinding = 1;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pBufferInfo = nullptr;
        descriptorWrites[3].pImageInfo = &imageInfo;
        descriptorWrites[3].pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(mManager->getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}

void TestDemo::initCommandBuffers() {
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

        //bind skybox pipeline
        vkCmdBindPipeline(mCommandBuffers[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mPipelineManager->getPipeline("skybox").getPipeline());

        //bind descriptor sets
        vkCmdBindDescriptorSets(mCommandBuffers[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                mPipelineManager->getPipeline("skybox").getPipelineLayout(),
                                0, // the index of the first descriptor set
                                1, // descriptor count
                                &mSkyboxDescriptor[i], // descriptor array
                                0, nullptr);

        mSkyboxModel->draw(mCommandBuffers[i]);

         //bind object pipeline
        vkCmdBindPipeline(mCommandBuffers[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mPipelineManager->getPipeline("object").getPipeline());

        vkCmdBindDescriptorSets(mCommandBuffers[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                mPipelineManager->getPipeline("object").getPipelineLayout(),
                                0, // the index of the first descriptor set
                                1, // descriptor count
                                &mObjectDescriptor[i], // descriptor array
                                0, nullptr);

        mObjectModel->draw(mCommandBuffers[i]);

        //end render pass
        mRenderPass->endRenderPass(mCommandBuffers[i]);

        //end command buffer
        Vk::endCommandBuffer(mCommandBuffers[i]);
    }
}

void TestDemo::initSyncObjects() {
    mSyncObjectPool->setup(mManager);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        mImageAvailableSemaphores.push_back(mSyncObjectPool->createSemaphore());
        mRenderFinishedSemaphores.push_back(mSyncObjectPool->createSemaphore());
        mInFlightFences.push_back(mSyncObjectPool->createFence());
    }
}

void TestDemo::init() {

    try {
        loadResources();
        initWindow();
        initManager();
        initDebugUtils();
        initSwapchain();
        initRenderPass();
        initDescriptorSetLayout();
        initShaders();
        initPipeline();
        initDepthStencilImage();
        initCommandPool();
        initFrameBuffers();
        initVertexBuffers();
        initUniformBuffers();
        initTexture();
        initDescriptorSets();
        initCommandBuffers();
        initSyncObjects();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::ends << std::endl;
    }
}

void TestDemo::destroy() {
    vkDeviceWaitIdle(mManager->getDevice());

    MX_FREE_OBJECT(mSyncObjectPool);
    MX_FREE_OBJECT(mCommandPool);

    vkDestroySampler(mManager->getDevice(), mCubeSampler, nullptr);

    vkDestroyImageView(mManager->getDevice(), mCubeImage.view, nullptr);
    vkDestroyImage(mManager->getDevice(), mCubeImage.image, nullptr);
    vkFreeMemory(mManager->getDevice(), mCubeImage.memory, nullptr);

    vkDestroyImageView(mManager->getDevice(), mDepthImage->view, nullptr);
    vkDestroyImage(mManager->getDevice(), mDepthImage->image, nullptr);
    vkFreeMemory(mManager->getDevice(), mDepthImage->memory, nullptr);

    for (auto& framebuffer : mFramebuffers)
        MX_FREE_OBJECT(framebuffer);

    MX_FREE_OBJECT(mShaderManager);
    MX_FREE_OBJECT(mSwapchain);
    MX_FREE_OBJECT(mDescriptorSetLayout);
    MX_FREE_OBJECT(mDescriptorPool);
    MX_FREE_OBJECT(mPipelineManager);
    MX_FREE_OBJECT(mRenderPass);

    MX_FREE_OBJECT(mSkyboxModel);
    MX_FREE_OBJECT(mObjectModel);
    for (auto& buffer : mObjectUniform)
        MX_FREE_OBJECT(buffer);

    for (auto& buffer : mSkyboxUniform)
        MX_FREE_OBJECT(buffer);

    MX_FREE_OBJECT(mDebug);
    MX_FREE_OBJECT(mManager);
    MX_FREE_OBJECT(mWindow);
}
