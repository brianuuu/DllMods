#include "AutodrawPatcher.h"

HOOK(void, __fastcall, ParseAutodrawData, 0xD151A0, uint32_t* This, void* Edx, float* a2)
{
    // Apparently this is called per-frame, why?
    if (This[42])
    {
        uint32_t* v5 = *(uint32_t**)(This + 42);
        uint32_t size = v5[3];
        char* pData = (char*)v5[5];
        printf("[AutodrawPatcher] Size = %u, address = 0x%08x\n", size, (uint32_t)pData);

        std::string str = "whip_gm_eyeL.uv-anim\nwhip_gm_eyeR.uv-anim\n";

        const size_t newSize = size + str.size();
        const std::unique_ptr<char[]> pBuffer = std::make_unique<char[]>(newSize);

        char* pNewData = pBuffer.get();
        memcpy(pNewData, pData, size);
        memmove(pNewData + str.size(), pNewData, size);
        memcpy(pNewData, str.c_str(), str.size());

        v5[3] = newSize;
        v5[5] = (uint32_t)pNewData;
        printf("[AutodrawPatcher] New size = %u, New address = 0x%08x\n", newSize, (uint32_t)pNewData);

        originalParseAutodrawData(This, Edx, a2);
    }
    else
    {
        originalParseAutodrawData(This, Edx, a2);
    }
}

void AutodrawPatcher::applyPatches()
{
    INSTALL_HOOK(ParseAutodrawData);
}
