#pragma once
#ifndef _MX_VK_SHADER_H_
#define _MX_VK_SHADER_H_

#include"MxVkComponent.h"

#include<fstream>
#include<vector>
#include<map>
#include<algorithm>
#include<string>

namespace Mix {
    namespace Vk {
        struct ShaderModule {
            VkShaderModule module;
            VkShaderStageFlagBits stage;

            ShaderModule() = default;
            ShaderModule(const VkShaderModule m, const VkShaderStageFlagBits s) {
                module = m;
                stage = s;
            }

            bool operator==(const ShaderModule& a);
        };

        class ShaderManager :public Component {
        private:
            std::map<std::string, ShaderModule> mModules;

        public:
            ShaderManager() {};
            bool setup(const Manager* manager);
            void createModule(const std::string& name, const std::vector<char>& code, const VkShaderStageFlagBits stage);
            void createModule(const std::string& name, const std::string& path, const VkShaderStageFlagBits stage);
            const ShaderModule& getModule(const std::string& name);
            bool destroyModule(const std::string& name);
            void destroy();
            ~ShaderManager() { destroy(); };
        };
    }
}

#endif // !_MX_VK_SHADER_H_
