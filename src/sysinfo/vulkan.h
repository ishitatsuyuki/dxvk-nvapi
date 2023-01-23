#pragma once

#include "../nvapi_private.h"

namespace dxvk {
    class Vulkan {

      public:
        Vulkan();
        explicit Vulkan(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr);
        virtual ~Vulkan();

        [[nodiscard]] virtual bool IsAvailable() const;
        [[nodiscard]] virtual std::set<std::string> GetDeviceExtensions(VkInstance vkInstance, VkPhysicalDevice vkDevice) const;
        virtual void GetPhysicalDeviceProperties2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceProperties2* deviceProperties2) const;
        virtual void GetPhysicalDeviceMemoryProperties2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceMemoryProperties2* memoryProperties2) const;

        [[nodiscard]] static NV_GPU_TYPE ToNvGpuType(VkPhysicalDeviceType vkDeviceType);

      private:
        PFN_vkGetInstanceProcAddr m_vkGetInstanceProcAddr{};

        template <typename T>
        T GetInstanceProcAddress(VkInstance vkInstance, const char* name) const;
    };
}
