#include "RankRunAnimation.h"
#include "Configuration.h"
#include "AnimationSetPatcher.h"

#define STH2006_RUN_STAGE_COUNT 4
const char* sth2006RunStageIDs[STH2006_RUN_STAGE_COUNT] = { "ghz200", "sph200", "ssh200", "euc200" };
bool RankRunAnimation::checkCanPlayRunAnimation()
{
    // Not Modern Sonic or currently in super form
    if (!*pModernSonicContext || Common::IsPlayerSuper() || Common::IsCurrentStageMission()) return false;

    switch (Configuration::Sonic::m_run)
    {
        case Configuration::RunResultType::EnableAll:
        {
            return true;
        }
        case Configuration::RunResultType::STH2006:
        {
            // Only enable for STH2006 Project stages
            for (int i = 0; i < STH2006_RUN_STAGE_COUNT; i++)
            {
                if (Common::CheckCurrentStage(sth2006RunStageIDs[i]))
                {
                    return true;
                }
            }
            break;
        }
        case Configuration::RunResultType::Custom:
        {
            // Only enable for custom defined stages
            for (std::string const& stage : Configuration::Sonic::m_runStages)
            {
                if (Common::CheckCurrentStage(stage.c_str()))
                {
                    return true;
                }
            }
            break;
        }
    }

    return false;
}

HOOK(void, __fastcall, MsgChangeResultState, 0xE692C0, void* This, void* Edx, uint32_t a2)
{
    uint32_t const state = *(uint32_t*)(a2 + 16);
    if (RankRunAnimation::checkCanPlayRunAnimation())
    {
        if (state == 1)
        {
            Common::SonicContextChangeAnimation(AnimationSetPatcher::RunResult);
        }
        else if (state == 3)
        {
            // Skip the normal rank animation
            return;
        }
    }

    originalMsgChangeResultState(This, Edx, a2);
}

void RankRunAnimation::applyPatches()
{
    if (Configuration::m_model != Configuration::ModelType::Sonic) return;
    if (Configuration::Sonic::m_run == Configuration::RunResultType::Disable) return;

    // Play animation when screen faded to white
    // Ignore original change animation during result screen
    INSTALL_HOOK(MsgChangeResultState);
}
