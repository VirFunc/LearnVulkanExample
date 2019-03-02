#include "MxVkComponent.h"

namespace Mix {
    namespace Vk {
        Component::Component() :mIsReady(false), mManager(nullptr) {
        }

        bool Component::setup(const Manager * manager) {
            if (mIsReady)
                destroy();
            mManager = manager;
            mIsReady = true;
            return true;
        }

        void Component::destroy() {
            if (!mIsReady)
                return;
            mManager = nullptr;
            mIsReady = false;
        }


        Component::~Component() {
            destroy();
        }
    }
}
