#include"MxVulkan.h"
#include"MxCamera.h"
#include"MxWindow.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/gtx/quaternion.hpp>
#include<stb/stb_image.h>
#include<array>
#include<vector>
#include<chrono>

#define MAX_FRAMES_IN_FLIGHT 2
#define FIXED_UPDATE_TIMESTEP (1.0f/60.0f)
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 680

namespace Vk = Mix::Vk;

struct UniformBufferObj {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 normal;

    glm::vec4 viewPos;
};

// test area --------------
// public methods

// test area --------------

class TestDemo {
private:
    Mix::MxWindow* mWindow;
    Vk::Manager* mManager;
    Vk::Debug* mDebug;
    Vk::ShaderManager* mShaderManager;
    Vk::Swapchain* mSwapchain;
    Vk::RenderPass* mRenderPass;
    Vk::DescriptorSetLayout* mDescriptorSetLayout;
    Vk::MxVkDescriptorPool* mDescriptorPool;
    Vk::PipelineManager* mPipelineManager;
    Vk::Pipeline* mPipeline;
    Vk::CommandPool* mCommandPool;
    std::vector<Vk::Framebuffer*> mFramebuffers;
    Vk::SyncObjectManager* mSyncObjectPool;

    VkSampleCountFlagBits mSampleCount;
    VkViewport mViewport;
    VkRect2D mScissor;
    Vk::Image* mDepthImage;

    std::vector<Vk::Buffer*> mSkyboxUniform;
    std::vector<Vk::Buffer*> mObjectUniform;
    std::vector<VkDescriptorSet> mSkyboxDescriptor;
    std::vector<VkDescriptorSet> mObjectDescriptor;
    std::vector<VkCommandBuffer> mCommandBuffers;
    std::vector<VkSemaphore> mImageAvailableSemaphores;
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    std::vector<VkFence> mInFlightFences;

    size_t mCurrFrame;

    Vk::ModelImporter* mModelImporter;
    Vk::Model* mSkyboxModel;
    Vk::Model* mObjectModel;

    Mix::Camera mCamera;

    const Uint8* mKeyboardState;

    Vk::Image mCubeImage;
    VkSampler mCubeSampler;

    void loadResources();
    void setupRs();
    void updateScene(const float deltaTime);
    void updateUniformBuffer(const uint32_t imageIndex);
    void draw();

    void keyboardStateEvent(const float deltaTime);
    void mouseMoveEvent(SDL_Event& e, const float deltaTime);

    void initWindow();
    void initManager();
    void initDebugUtils();
    void initSwapchain();
    void initRenderPass();
    void initDescriptorSetLayout();
    void initShaders();
    void initPipeline();
    void initDepthStencilImage();
    void initCommandPool();
    void initFrameBuffers();
    void initVertexBuffers();
    void initUniformBuffers();
    void initTexture();
    void initDescriptorSets();
    void initCommandBuffers();
    void initSyncObjects();
public:
    TestDemo();
    ~TestDemo() = default;
    void init();
    void run();
    void destroy();
};
