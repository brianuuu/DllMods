#include "RankQuote.h"

ResultData m_resultData;
HOOK(int, __fastcall, RankQuote_CStateGoalFadeBeforeBegin, 0xCFE080, uint32_t* This)
{
    int result = originalRankQuote_CStateGoalFadeBeforeBegin(This);
    {
        m_resultData = *(ResultData*)(This[2] + 0x16C);
    }
    return result;
}

void PlayRankQuote(uint32_t This)
{
    uint32_t voiceCueID = -1;
    uint32_t slamCueID = -1;
    switch (m_resultData.m_perfectRank)
    {
    case ResultRankType::S: voiceCueID = 40000; slamCueID = 1000041; printf("[Rank Quote] Rank = S"); break;
    case ResultRankType::A: voiceCueID = 40001; slamCueID = 1000042; printf("[Rank Quote] Rank = A"); break;
    case ResultRankType::B: voiceCueID = 40002; slamCueID = 1000043; printf("[Rank Quote] Rank = B"); break;
    case ResultRankType::C: voiceCueID = 40003; slamCueID = 1000044; printf("[Rank Quote] Rank = C"); break;
    case ResultRankType::D: voiceCueID = 40004; slamCueID = 1000045; printf("[Rank Quote] Rank = D"); break;
    default: voiceCueID = 40005; slamCueID = 1000046; printf("[Rank Quote] Rank = E"); break;
    }

    static SharedPtrTypeless rankVoiceHandle;
    Common::PlaySoundStatic(rankVoiceHandle, voiceCueID);

    static SharedPtrTypeless rankSoundHandle;
    Common::PlaySoundStatic(rankSoundHandle, slamCueID);
}

HOOK(void, __fastcall, RankQuote_ChangeRank, 0x10B76D0, uint32_t* This)
{
    WRITE_MEMORY(0x11D237A, int, -1);
    PlayRankQuote(This[2]);

    originalRankQuote_ChangeRank(This);
    WRITE_MEMORY(0x11D237A, uint32_t, 1010002);
}

HOOK(void, __fastcall, RankQuote_ShowRank, 0x10B7800, uint32_t* This)
{
    FUNCTION_PTR(bool, __cdecl, IsPerfectBonus, 0x10B8A90);
    if (!IsPerfectBonus())
    {
        WRITE_MEMORY(0x11D237A, int, -1);
        PlayRankQuote(This[2]);
    }

    originalRankQuote_ShowRank(This);
    WRITE_MEMORY(0x11D237A, uint32_t, 1010002);
}

void RankQuote::applyPatches()
{
    INSTALL_HOOK(RankQuote_CStateGoalFadeBeforeBegin);
    INSTALL_HOOK(RankQuote_ChangeRank);
    INSTALL_HOOK(RankQuote_ShowRank);
}
