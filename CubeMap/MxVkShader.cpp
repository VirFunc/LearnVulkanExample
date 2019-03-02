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

        void ShaderManager::createModule(const std::string& name, const std::vector<char>& code, const VkShaderStageFlagBits stage) {
            if (mModules.count(name) != 0)
                throw std::runtime_error("Error : [" + name + "] already exists");

            VkShaderModule temp;
            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

            MX_VK_CHECK_RESULT(vkCreateShaderModule(mManager->getDevice(), &createInfo, nullptr, &temp));
            mModules[name] = { temp,stage };
        }

        void ShaderManager::createModule(const std::string& name, const std::string & path, const VkShaderStageFlagBits stage) {
            if (mModules.count(name) != 0)
                throw std::runtime_error("Error : [" + name + "] already exists");

            std::ifstream inFile(path, std::ios_base::ate | std::ios_base::binary);
            if (!inFile.is_open())
                throw std::runtime_error("Error : Failed to open file");
            size_t fileSize = static_cast<size_t>(inFile.tellg());
            std::vector<char> buffer(fileSize);
            inFile.seekg(std::ios_base::beg);
            inFile.read(buffer.data(), fileSize);
            inFile.close();

            createModule(name, buffer, stage);
        }

        const ShaderModule & ShaderManager::getModule(const std::string & name) {
            if (mModules.count(name) == 0)
                throw std::runtime_error("Error : [" + name + "] does not exist");

            return mModules[name];
        }

        bool ShaderManager::destroyModule(const std::string& name) {
            if (mModules.count(name) == 0)
                throw std::runtime_error("Error : [" + name + "] does not exist");

            vkDestroyShaderModule(mManager->getDevice(), mModules[name].module, nullptr);
            mModules.erase(name);
            return true;
        }

        void ShaderManager::destroy() {
            if (!mIsReady)
                return;
            for (auto& pair : mModules) {
                vkDestroyShaderModule(mManager->getDevice(), pair.second.module, nullptr);
            }
            mManager = nullptr;
            mIsReady = false;
        }
    }
}