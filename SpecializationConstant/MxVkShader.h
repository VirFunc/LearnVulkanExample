#pragma once
#ifndef _MX_VK_SHADER_H_
#define _MX_VK_SHADER_H_

#include"MxVkComponent.h"

#include<fstream>
#include<vector>
#include<list>
#include<algorithm>
#include<string>

namespace Mix {
    namespace Vk {
        struct ShaderModule {
            VkShaderModule module;
            VkShaderStageFlagBits stage;
            bool operator==(const ShaderModule& a);
        };

        class ShaderManager :public Component {
        private:
            std::list<ShaderModule> mModules;

        public:
            using ShaderModuleIterator = std::list<ShaderModule>::const_iterator;
            ShaderManager() {};
            bool setup(const Manager* manager);
            ShaderModuleIterator createModule(const std::vector<char>& code, const VkShaderStageFlagBits stage);
            ShaderModuleIterator createModule(const std::string& path, const VkShaderStageFlagBits stage);
            bool destroyModule(ShaderModuleIterator it);
            void destroy();
            ~ShaderManager() { destroy(); };
        };
    }
}

#endif // !_MX_VK_SHADER_H_
