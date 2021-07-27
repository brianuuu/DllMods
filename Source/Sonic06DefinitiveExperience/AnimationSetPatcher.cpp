#include "AnimationSetPatcher.h"
#include "Configuration.h"

std::vector<NewAnimationData> AnimationSetPatcher::m_newAnimationData;

HOOK(void*, __cdecl, InitializeSonicAnimationList, 0x1272490)
{
    void* pResult = originalInitializeSonicAnimationList();
    {
        CAnimationStateSet* pList = (CAnimationStateSet*)0x15E8D40;
        CAnimationStateInfo* pEntries = new CAnimationStateInfo[pList->m_Count + AnimationSetPatcher::m_newAnimationData.size()];
        std::copy(pList->m_pEntries, pList->m_pEntries + pList->m_Count, pEntries);

        for (size_t i = 0; i < AnimationSetPatcher::m_newAnimationData.size(); i++)
        {
            NewAnimationData const& data = AnimationSetPatcher::m_newAnimationData[i];

            pEntries[pList->m_Count + i].m_Name = data.m_stateName;
            pEntries[pList->m_Count + i].m_FileName = data.m_fileName;
            pEntries[pList->m_Count + i].m_Speed = data.m_speed;
            pEntries[pList->m_Count + i].m_PlaybackType = !data.m_isLoop;
            pEntries[pList->m_Count + i].field10 = 0;
            pEntries[pList->m_Count + i].field14 = -1.0f;
            pEntries[pList->m_Count + i].field18 = -1.0f;
            pEntries[pList->m_Count + i].field1C = 0;
            pEntries[pList->m_Count + i].field20 = -1;
            pEntries[pList->m_Count + i].field24 = -1;
            pEntries[pList->m_Count + i].field28 = -1;
            pEntries[pList->m_Count + i].field2C = -1;
        }

        WRITE_MEMORY(&pList->m_pEntries, void*, pEntries);
        WRITE_MEMORY(&pList->m_Count, size_t, pList->m_Count + AnimationSetPatcher::m_newAnimationData.size());
    }

    return pResult;
}

FUNCTION_PTR(void*, __stdcall, fpCreateAnimationState, 0xCDFA20, void* This, boost::shared_ptr<void>& spAnimationState, const Hedgehog::Base::CSharedString& name, const Hedgehog::Base::CSharedString& name2);
FUNCTION_PTR(uint32_t*, __stdcall, fpGetAnimationTransitionData, 0xCDFB40, void* A2, const Hedgehog::Base::CSharedString& name);
HOOK(void, __fastcall, CSonicCreateAnimationStates, 0xE1B6C0, void* This, void* Edx, void* A2, void* A3)
{
    boost::shared_ptr<void> spAnimationState;
    for (NewAnimationData const& data : AnimationSetPatcher::m_newAnimationData)
    {
        fpCreateAnimationState(A2, spAnimationState, data.m_stateName, data.m_stateName);
    }

    // Set transition data
    for (NewAnimationData const& data : AnimationSetPatcher::m_newAnimationData)
    {
        if (!data.m_destinationState) continue;

        // Initialise data on destination state
        bool found = false;
        for (NewAnimationData const& destData : AnimationSetPatcher::m_newAnimationData)
        {
            if (data.m_destinationState == destData.m_stateName)
            {
                uint32_t* pTransitionDestData = fpGetAnimationTransitionData(A2, destData.m_stateName);
                *(uint64_t*)(*pTransitionDestData + 96) = 1;
                *(uint64_t*)(*pTransitionDestData + 104) = 0;
                *(uint32_t*)(*pTransitionDestData + 112) = 1;
                found = true;
                break;
            }
        }

        if (found)
        {
            uint32_t* pTransitionData = fpGetAnimationTransitionData(A2, data.m_stateName);
            *(uint64_t*)(*pTransitionData + 96) = 1;
            *(uint64_t*)(*pTransitionData + 104) = 0;
            *(uint32_t*)(*pTransitionData + 112) = 1;
            *(float*)(*pTransitionData + 140) = -1.0f;
            *(bool*)(*pTransitionData + 144) = true;
            *(Hedgehog::Base::CSharedString*)(*pTransitionData + 136) = data.m_destinationState;
        }
        else
        {
            MessageBox(NULL, L"Animation transition destination does not exist, please check your code!", NULL, MB_ICONERROR);
        }
    }

    originalCSonicCreateAnimationStates(This, Edx, A2, A3);
}

void AnimationSetPatcher::applyPatches()
{
    if (Configuration::m_model == Configuration::ModelType::Sonic &&
        Configuration::m_run != Configuration::RunResultType::Disable)
    {
        static const char* volatile const RunResult = "RunResult";
        static const char* volatile const RunResultLoop = "RunResultLoop";
        m_newAnimationData.emplace_back(RunResult, "sn_result_run", 1.0f, false, RunResultLoop);
        m_newAnimationData.emplace_back(RunResultLoop, "sn_result_run_loop", 1.0f, true, nullptr);
    }

    if (!m_newAnimationData.empty())
    {
        INSTALL_HOOK(InitializeSonicAnimationList);
        INSTALL_HOOK(CSonicCreateAnimationStates);
    }
}
