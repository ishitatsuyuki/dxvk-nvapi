#include "nvapi_private.h"
#include "nvapi_static.h"
#include "util/util_statuscode.h"
#include "../version.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_SYS_GetPhysicalGpuFromDisplayId(NvU32 displayId, NvPhysicalGpuHandle *hPhysicalGpu) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        auto output = nvapiAdapterRegistry->GetOutput(displayId);
        if (output == nullptr)
            return InvalidArgument(n);

        *hPhysicalGpu = (NvPhysicalGpuHandle) output->GetParent();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pDriverVersion == nullptr || szBuildBranchString == nullptr)
            return InvalidArgument(n);

        auto version = nvapiAdapterRegistry->GetAdapter()->GetDriverVersion();
        *pDriverVersion = version;
        strcpy(szBuildBranchString, str::format("r", (version / 100), "_", DXVK_NVAPI_VERSION).c_str());

        return Ok(n);
    }
}
