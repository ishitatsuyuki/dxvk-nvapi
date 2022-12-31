#include "../util/util_log.h"
#include "nvapi_d3d_instance.h"

namespace dxvk {
    NvapiD3dInstance::NvapiD3dInstance(ResourceFactory& resourceFactory)
        : m_resourceFactory(resourceFactory) {}

    NvapiD3dInstance::~NvapiD3dInstance() = default;

    void NvapiD3dInstance::Initialize() {
        m_lfx = m_resourceFactory.CreateLfx();
    }

    bool NvapiD3dInstance::IsReflexAvailable() {
        return m_lfx->IsAvailable();
    }

    Lfx2* NvapiD3dInstance::GetLfx2Instance() const {
        return &*m_lfx;
    }

    bool NvapiD3dInstance::IsReflexEnabled() const {
        return m_isLfxEnabled;
    }

    void NvapiD3dInstance::SetReflexEnabled(bool value) {
        m_isLfxEnabled = value;
    }

    bool NvapiD3dInstance::UseLatencyMarkers() const {
        return m_useLatencyMarkers;
    }

    void NvapiD3dInstance::SetUseLatencyMarkers(bool value) {
        m_useLatencyMarkers = value;
    }
}