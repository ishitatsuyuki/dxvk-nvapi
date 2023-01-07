#include "lfx2.h"
#include "util/util_log.h"
#include "util/util_string.h"

namespace dxvk {

    Lfx2::Lfx2() {
        const auto lfxModuleName = "latencyflex2_rust.dll";

        m_lfxModule = ::LoadLibraryA(lfxModuleName);
        if (m_lfxModule == nullptr) {
            auto lastError = ::GetLastError();
            if (lastError != ERROR_MOD_NOT_FOUND) // Ignore library not found
                log::write(str::format("Loading ", lfxModuleName,
                    " failed with error code: ", lastError));
            return;
        }

#define LOAD_PFN(x) \
    this->x = GetProcAddress<decltype(&::lfx2##x)>("lfx2" #x)

        LOAD_PFN(ContextCreate);
        LOAD_PFN(ContextAddRef);
        LOAD_PFN(ContextRelease);
        LOAD_PFN(FrameCreate);
        LOAD_PFN(FrameAddRef);
        LOAD_PFN(FrameRelease);
        LOAD_PFN(MarkSection);
        LOAD_PFN(SleepUntil);
        LOAD_PFN(TimestampNow);
#ifdef _WIN32
        LOAD_PFN(TimestampFromQpc);
#endif
        LOAD_PFN(ImplicitContextCreate);
        LOAD_PFN(ImplicitContextRelease);
        LOAD_PFN(ImplicitContextReset);
        LOAD_PFN(FrameCreateImplicit);
        LOAD_PFN(FrameDequeueImplicit);

#undef LOAD_PFN

        m_lfxContext = ContextCreate();
    }

    Lfx2::~Lfx2() {
        if (m_lfxContext)
            ContextRelease(m_lfxContext);

        if (m_lfxModule == nullptr)
            return;

        ::FreeLibrary(m_lfxModule);
        m_lfxModule = nullptr;
    }

    bool Lfx2::IsAvailable() const {
        return m_lfxModule != nullptr;
    }

    void Lfx2::Sleep() {
        std::unique_lock<std::mutex> lock(m_frameMapMutex);
        if (!m_nextFrame) {
            lfx2Timestamp sleepTarget;
            m_nextFrame = FrameCreate(m_lfxContext, &sleepTarget);
            lock.unlock();

            SleepUntil(sleepTarget);
        }
        // Sleep called without recording frame timing, skip
    }

    void Lfx2::Mark(uint64_t frame_id, NV_LATENCY_MARKER_TYPE type, Com<ID3D11VkExtContext2>& d3d11Context) {
        log::write(str::format("mark, frame_id: ", frame_id, ", type: ", type));

        uint32_t section;
        lfx2MarkType markType;
        switch (type) {
            case SIMULATION_START:
                section = 0;
                markType = lfx2MarkType::lfx2MarkTypeBegin;
                break;
            case SIMULATION_END:
                section = 0;
                markType = lfx2MarkType::lfx2MarkTypeEnd;
                break;
            case RENDERSUBMIT_START:
                section = 500;
                markType = lfx2MarkType::lfx2MarkTypeBegin;
                break;
            case RENDERSUBMIT_END:
                section = 500;
                markType = lfx2MarkType::lfx2MarkTypeEnd;
                break;
            default:
                return;
        }

        std::unique_lock<std::mutex> lock(m_frameMapMutex);
        lfx2Frame* frame;
        if (type == SIMULATION_START) {
            EnsureFrame();
            m_frameMap[frame_id] = m_nextFrame;
            m_callsExpectedByFrame[frame_id] = 4;
            frame = m_nextFrame;
            m_nextFrame = nullptr;

            decltype(m_frameMap)::iterator it;
            if (frame_id >= kMaxInflightFrames && (it = m_frameMap.upper_bound(frame_id - kMaxInflightFrames)) != m_frameMap.end()) {
                for (auto i = m_frameMap.begin(); i != it;) {
                    FrameRelease(i->second);
                    m_callsExpectedByFrame.erase(i->first);
                    i = m_frameMap.erase(i);
                    log::write(str::format("forced destroy, frame_id: ", frame_id, ", type: ", type));
                }
            }
        } else {
            if (m_frameMap.find(frame_id) == m_frameMap.end())
                return;
            frame = m_frameMap[frame_id];
        }
        lock.unlock();

        MarkSection(frame, section, markType, TimestampNow());

        if (d3d11Context.ptr()) {
            if (type == RENDERSUBMIT_START) {
                d3d11Context->MarkRenderStartLFX2((void*)frame);
            } else if (type == RENDERSUBMIT_END) {
                d3d11Context->MarkRenderEndLFX2((void*)frame);
            }
        }

        lock.lock();
        if (--m_callsExpectedByFrame[frame_id] == 0) {
            FrameRelease(frame);
            m_frameMap.erase(frame_id);
            m_callsExpectedByFrame.erase(frame_id);
            log::write(str::format("destroy, frame_id: ", frame_id, ", type: ", type));
        }
    }

    void Lfx2::EnsureFrame() {
        if (!m_nextFrame) {
            lfx2Timestamp sleepTarget;
            m_nextFrame = FrameCreate(m_lfxContext, &sleepTarget);
        }
    }

    void Lfx2::SleepImplicit(Com<ID3D11VkExtDevice2>& d3d11Device) {
        lfx2Timestamp sleepTarget;
        lfx2Frame* implicitFrame = FrameCreateImplicit(static_cast<lfx2ImplicitContext*>(d3d11Device->GetImplicitContextLFX2()), &sleepTarget);

        SleepUntil(sleepTarget);
        MarkSection(implicitFrame, 0, lfx2MarkType::lfx2MarkTypeBegin, TimestampNow());
        MarkSection(implicitFrame, 0, lfx2MarkType::lfx2MarkTypeEnd, TimestampNow());
        FrameRelease(implicitFrame);
    }

    void Lfx2::SleepImplicit(Com<ID3D12DeviceLfx2>& d3d12Device) {
        lfx2Timestamp sleepTarget;
        lfx2Frame* implicitFrame;

        d3d12Device->EnqueueFrameLFX2(&sleepTarget, reinterpret_cast<void**>(&implicitFrame));
        SleepUntil(sleepTarget);
        MarkSection(implicitFrame, 0, lfx2MarkType::lfx2MarkTypeBegin, TimestampNow());
        MarkSection(implicitFrame, 0, lfx2MarkType::lfx2MarkTypeEnd, TimestampNow());
        FrameRelease(implicitFrame);
    }

    template <typename T>
    T Lfx2::GetProcAddress(const char* name) {
        return reinterpret_cast<T>(reinterpret_cast<void*>(::GetProcAddress(m_lfxModule, name)));
    }
}