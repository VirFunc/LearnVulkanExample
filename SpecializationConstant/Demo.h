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
#include<array>
#include<vector>
#include<chrono>

#define MAX_FRAMES_IN_FLIGHT 2
#define FIXED_UPDATE_TIMESTEP (1.0f/60.0f)

namespace Vk = Mix::Vk;

struct UniformBufferObj {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
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
    Vk::Pipeline* mPipeline;
    Vk::CommandPool* mCommandPool;
    std::vector<Vk::Framebuffer*> mFramebuffers;
    Vk::SyncObjectManager* mSyncObjectPool;

    VkSampleCountFlagBits mSampleCount;
    VkViewport mViewport;
    VkRect2D mScissor;
    Vk::Image* mDepthImage;

    Vk::Buffer* mVertexBuffer;
    std::vector<Vk::Buffer*> mUniformBuffers;
    std::vector<VkDescriptorSet> mDescriptorSets;
    std::vector<VkCommandBuffer> mCommandBuffers;
    std::vector<VkSemaphore> mImageAvailableSemaphores;
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    std::vector<VkFence> mInFlightFences;

    size_t mCurrFrame;

    Vk::ModelImporter* mModelImporter;
    Vk::Model* mModel;

    Mix::Camera mCamera;

// test area --------------
private:

// test area --------------

    void draw();
    void updateUniformBuffer(const uint32_t imageIndex);
public:
    TestDemo();
    ~TestDemo() = default;
    bool init();
    void run();
    void destroy();

// test area --------------
// public methods
public:

// test area --------------
};
