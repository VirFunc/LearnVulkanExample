#include "MxVkUtils.h"

namespace Mix {
    namespace Vk {
        std::string VkResultToString(const VkResult res) {
            std::string r;
            switch (res) {
            case VK_SUCCESS:
                break;
            case VK_NOT_READY:r = "VK_NOT_READY"; break;
            case VK_TIMEOUT:r = "VK_TIMEOUT"; break;
            case VK_EVENT_SET:r = "VK_EVENT_SET"; break;
            case VK_EVENT_RESET:r = "VK_EVENT_RESET"; break;
            case VK_INCOMPLETE:r = "VK_INCOMPLETE"; break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:r = "VK_ERROR_OUT_OF_HOST_MEMORY"; break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:r = "VK_ERROR_OUT_OF_DEVICE_MEMORY"; break;
            case VK_ERROR_INITIALIZATION_FAILED:r = "VK_ERROR_INITIALIZATION_FAILED"; break;
            case VK_ERROR_DEVICE_LOST:r = "VK_ERROR_DEVICE_LOST"; break;
            case VK_ERROR_MEMORY_MAP_FAILED:r = "VK_ERROR_MEMORY_MAP_FAILED"; break;
            case VK_ERROR_LAYER_NOT_PRESENT:r = "VK_ERROR_LAYER_NOT_PRESENT"; break;
            case VK_ERROR_EXTENSION_NOT_PRESENT:r = "VK_ERROR_EXTENSION_NOT_PRESENT"; break;
            case VK_ERROR_FEATURE_NOT_PRESENT:r = "VK_ERROR_FEATURE_NOT_PRESENT"; break;
            case VK_ERROR_INCOMPATIBLE_DRIVER:r = "VK_ERROR_INCOMPATIBLE_DRIVER"; break;
            case VK_ERROR_TOO_MANY_OBJECTS:r = "VK_ERROR_TOO_MANY_OBJECTS"; break;
            case VK_ERROR_FORMAT_NOT_SUPPORTED:r = "VK_ERROR_FORMAT_NOT_SUPPORTED"; break;
            case VK_ERROR_FRAGMENTED_POOL:r = "VK_ERROR_FRAGMENTED_POOL"; break;
            case VK_ERROR_OUT_OF_POOL_MEMORY:r = "VK_ERROR_OUT_OF_POOL_MEMORY"; break;
            case VK_ERROR_INVALID_EXTERNAL_HANDLE:r = "VK_ERROR_INVALID_EXTERNAL_HANDLE"; break;
            case VK_ERROR_SURFACE_LOST_KHR:r = "VK_ERROR_SURFACE_LOST_KHR"; break;
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:r = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"; break;
            case VK_SUBOPTIMAL_KHR:r = "VK_SUBOPTIMAL_KHR"; break;
            case VK_ERROR_OUT_OF_DATE_KHR:r = "VK_ERROR_OUT_OF_DATE_KHR"; break;
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:r = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR"; break;
            case VK_ERROR_VALIDATION_FAILED_EXT:r = "VK_ERROR_VALIDATION_FAILED_EXT"; break;
            case VK_ERROR_INVALID_SHADER_NV:r = "VK_ERROR_INVALID_SHADER_NV"; break;
            case VK_ERROR_FRAGMENTATION_EXT:r = "VK_ERROR_FRAGMENTATION_EXT"; break;
            case VK_ERROR_NOT_PERMITTED_EXT:r = "VK_ERROR_NOT_PERMITTED_EXT"; break;
            default:r = "UNKNOWN RESULT"; break;
            }
            return r;
        }
    }
}
