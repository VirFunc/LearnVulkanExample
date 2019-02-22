#pragma once
#ifndef _MX_VK_COMPONENT_H_
#define _MX_VK_COMPONENT_H_

#include"MxVkManager.h"

namespace Mix {
    namespace Vk {
        class Component {
        protected:
            bool mIsReady;
            const Manager* mManager;
        public:
            Component();
            virtual bool setup(const Manager* manager);
            virtual void destroy();
            virtual ~Component();
        };
    }
}

#endif // !_MX_VK_OBJECT_H_