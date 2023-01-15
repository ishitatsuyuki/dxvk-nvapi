#pragma once

#include "../nvapi_private.h"
#include "../util/com_pointer.h"

namespace dxvk {
    class NvapiAdapter;

    class NvapiOutput {

      public:
        explicit NvapiOutput(NvapiAdapter* parent, uint32_t adapterIndex, uint32_t outputIndex);
        ~NvapiOutput();

        void Initialize(Com<IDXGIOutput>& dxgiOutput);
        [[nodiscard]] NvapiAdapter* GetParent() const;
        [[nodiscard]] uint32_t GetId() const;
        [[nodiscard]] std::string GetDeviceName() const;
        [[nodiscard]] bool IsPrimary() const;

      private:
        NvapiAdapter* m_parent;
        uint32_t m_id;
        std::string m_deviceName;
        bool m_isPrimary{};
    };
}
