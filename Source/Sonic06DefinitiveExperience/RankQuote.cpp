#include "RankQuote.h"
#include "RankRunAnimation.h"
#include "VoiceOver.h"

HOOK(void, __fastcall, RankQuote_MsgChangeResultState, 0xE692C0, void* This, void* Edx, uint32_t a2)
{
    uint32_t const state = *(uint32_t*)(a2 + 16);
    if (state == 3 
    && !RankRunAnimation::checkCanPlayRunAnimation()
    && !Common::IsPlayerSuper())
    {
        // Play character stage complete voice
        VoiceOver::playStageCompleteVoice();
    }

    originalRankQuote_MsgChangeResultState(This, Edx, a2);
}

void RankQuote::applyPatches()
{
    // Play character stage complete voice
    INSTALL_HOOK(RankQuote_MsgChangeResultState);
}
