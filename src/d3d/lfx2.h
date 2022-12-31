#pragma once

#include "../inc/latencyflex2.h"
#include "../nvapi_private.h"
#include "dxvk/dxvk_interfaces.h"
#include "util/com_pointer.h"

namespace dxvk {
    class Lfx2 {
      public:
        Lfx2();
        virtual ~Lfx2();

        [[nodiscard]] virtual bool IsAvailable() const;
        virtual void Sleep();
        virtual void Mark(uint64_t frame_id, NV_LATENCY_MARKER_TYPE type, Com<ID3D11VkExtContext2> &d3d11Context);

      private:
#define DECLARE_PFN(x) \
    decltype(&::lfx2##x) x {}

        DECLARE_PFN(ContextCreate);
        DECLARE_PFN(ContextAddRef);
        DECLARE_PFN(ContextRelease);
        DECLARE_PFN(FrameCreate);
        DECLARE_PFN(FrameAddRef);
        DECLARE_PFN(FrameRelease);
        DECLARE_PFN(MarkSection);
        DECLARE_PFN(SleepUntil);
        DECLARE_PFN(TimestampNow);

#undef DECLARE_PFN

        template <typename T>
        T GetProcAddress(const char* name);

        void EnsureFrame();

        std::mutex m_frameMapMutex;
        std::map<uint64_t, const lfx2Frame*> m_frameMap;
        std::unordered_map<uint64_t, uint32_t> m_callsExpectedByFrame;

        HMODULE m_lfxModule{};
        const lfx2Context* m_lfxContext{};
        const lfx2Frame* m_nextFrame{};

        static constexpr uint64_t kMaxInflightFrames = 64;
    };
}