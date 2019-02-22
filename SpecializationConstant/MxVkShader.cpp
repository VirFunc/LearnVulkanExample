#include "MxVkShader.h"

namespace Mix {
    namespace Vk {
        bool ShaderModule::operator==(const ShaderModule& a) {
            return module == a.module;
        }

        bool ShaderManager::setup(const Manager* manager) {
            if (mIsReady)
                destroy();

            mManager = manager;
            mIsReady = true;
            return true;
        }

        ShaderManager::ShaderModuleIterator ShaderManager::createModule(const std::vector<char>& code, const VkShaderStageFlagBits stage) {
            VkShaderModule temp;
            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

            MX_VK_CHECK_RESULT(vkCreateShaderModule(mManager->getDevice(), &createInfo, nullptr, &temp));
            mModules.push_back({ temp,stage });
            return --mModules.end();
        }

        ShaderManager::ShaderModuleIterator ShaderManager::createModule(const std::string & path, const VkShaderStageFlagBits stage) {
            std::ifstream inFile(path, std::ios_base::ate | std::ios_base::binary);
            if (!inFile.is_open())
                throw std::runtime_error("Error : Failed to open file");
            size_t fileSize = static_cast<size_t>(inFile.tellg());
            std::vector<char> buffer(fileSize);
            inFile.seekg(std::ios_base::beg);
            inFile.read(buffer.data(), fileSize);
            inFile.close();
            return createModule(buffer, stage);
        }

        bool ShaderManager::destroyModule(ShaderModuleIterator it) {
            auto find = std::find(mModules.begin(), mModules.end(), *it);
            if (find == mModules.end())
                return false;
            vkDestroyShaderModule(mManager->getDevice(), it->module, nullptr);
            mModules.erase(it);
            return true;
        }

        void ShaderManager::destroy() {
            if (!mIsReady)
                return;
            for (auto& module : mModules) {
                vkDestroyShaderModule(mManager->getDevice(), module.module, nullptr);
            }
            mManager = nullptr;
            mIsReady = false;
        }
    }
}