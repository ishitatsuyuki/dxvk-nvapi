#pragma once

#include "../nvapi_private.h"
#include "../resource_factory.h"
#include "nvapi_adapter.h"
#include "nvapi_output.h"
#include "vulkan.h"
#include "nvml.h"

namespace dxvk {
    class NvapiAdapterRegistry {

      public:
        explicit NvapiAdapterRegistry(ResourceFactory& resourceFactory);
        ~NvapiAdapterRegistry();

        bool Initialize();

        [[nodiscard]] uint32_t GetAdapterCount() const;
        [[nodiscard]] NvapiAdapter* GetAdapter(uint32_t index) const;
        [[nodiscard]] NvapiAdapter* GetFirstAdapter() const;
        [[nodiscard]] NvapiAdapter* FindAdapter(const LUID& luid) const;
        [[nodiscard]] bool IsAdapter(NvapiAdapter* handle) const;

        [[nodiscard]] NvapiOutput* GetOutput(uint32_t index) const;
        [[nodiscard]] NvapiOutput* FindOutput(const std::string& displayName) const;
        [[nodiscard]] NvapiOutput* FindOutput(uint32_t id) const;
        [[nodiscard]] NvapiOutput* FindPrimaryOutput() const;
        [[nodiscard]] bool IsOutput(NvapiOutput* handle) const;

      private:
        ResourceFactory& m_resourceFactory;
        std::unique_ptr<Vulkan> m_vulkan;
        std::unique_ptr<Nvml> m_nvml;
        std::vector<NvapiAdapter*> m_nvapiAdapters;
        std::vector<NvapiOutput*> m_nvapiOutputs;
    };
}
