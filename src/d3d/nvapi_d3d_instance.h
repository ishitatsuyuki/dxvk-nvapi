#pragma once

#include "../nvapi_private.h"
#include "../resource_factory.h"

namespace dxvk {
    class NvapiD3dInstance {
      public:
        explicit NvapiD3dInstance(ResourceFactory& resourceFactory);
        ~NvapiD3dInstance();

        void Initialize();
        [[nodiscard]] bool IsReflexAvailable();
        [[nodiscard]] bool IsReflexEnabled() const;
        void SetReflexEnabled(bool value);
        [[nodiscard]] bool UseLatencyMarkers() const;
        void SetUseLatencyMarkers(bool value);
        [[nodiscard]] Lfx2 *GetLfx2Instance() const;

      private:
        ResourceFactory& m_resourceFactory;
        std::unique_ptr<Lfx2> m_lfx;
        bool m_isLfxEnabled = false;
        bool m_useLatencyMarkers;
    };
}