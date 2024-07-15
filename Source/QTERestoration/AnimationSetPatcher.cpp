#include "AnimationSetPatcher.h"
#include "Configuration.h"

FUNCTION_PTR(void*, __stdcall, fpCreateAnimationState, 0xCDFA20, void* This, boost::shared_ptr<void>& spAnimationState, const Hedgehog::Base::CSharedString& name, const Hedgehog::Base::CSharedString& name2);
FUNCTION_PTR(uint32_t*, __stdcall, fpGetAnimationTransitionData, 0xCDFB40, void* A2, const Hedgehog::Base::CSharedString& name);

HOOK(bool, __fastcall, CAnimationControlSingle_Debug, 0x6D84F0, uint32_t** This, void* Edx, float a2, int a3)
{
    std::string name((char*)(This[58][2]));
    if (name.find("sn_") != string::npos)
    {
        printf("%s\n", name.c_str());
    }
    return originalCAnimationControlSingle_Debug(This, Edx, a2, a3);
}

HOOK(void, __stdcall, CSonicContextChangeAnimation, 0xCDFC80, void* This, int a2, Hedgehog::Base::CSharedString& name)
{
    printf("[AnimationManager] Animation change: %s\n", name.c_str());
    originalCSonicContextChangeAnimation(This, a2, name);
}

//---------------------------------------------------
// CSonic
//---------------------------------------------------
std::vector<NewAnimationData> AnimationSetPatcher::m_newAnimationData;
HOOK(void*, __cdecl, InitializeSonicAnimationList, 0x1272490)
{
    void* pResult = originalInitializeSonicAnimationList();
    {
        CAnimationStateSet* pList = (CAnimationStateSet*)0x15E8D40;
        size_t newCount = pList->m_Count + AnimationSetPatcher::m_newAnimationData.size();
        CAnimationStateInfo* pEntries = new CAnimationStateInfo[newCount];
        std::copy(pList->m_pEntries, pList->m_pEntries + pList->m_Count, pEntries);

        AnimationSetPatcher::initializeAnimationList(pEntries, pList->m_Count, AnimationSetPatcher::m_newAnimationData);
        WRITE_MEMORY(&pList->m_pEntries, void*, pEntries);
        WRITE_MEMORY(&pList->m_Count, size_t, newCount);
    }

    return pResult;
}

HOOK(void, __fastcall, CSonicCreateAnimationStates, 0xE1B6C0, void* This, void* Edx, void* A2, void* A3)
{
    AnimationSetPatcher::createAnimationState(A2, AnimationSetPatcher::m_newAnimationData);
    originalCSonicCreateAnimationStates(This, Edx, A2, A3);
}

//---------------------------------------------------
// CSonicSpRenderableSsn
//---------------------------------------------------
CAnimationStateInfo* SuperSonicAnimationList = nullptr;
uint32_t SuperSonicAnimationListSize = 0;
void __declspec(naked) UpdateSuperSonicAnimationListSize()
{
    static uint32_t returnAddress = 0xDA31CF;
    __asm
    {
        push    SuperSonicAnimationListSize
        push    SuperSonicAnimationList
        jmp     [returnAddress]
    }
}

std::vector<NewAnimationData> AnimationSetPatcher::m_newAnimationDataSuper;
HOOK(void*, __cdecl, InitializeSuperSonicAnimationList, 0x1291D60)
{
    void* pResult = originalInitializeSuperSonicAnimationList();
    {
        CAnimationStateInfo* pEntriesOriginal = (CAnimationStateInfo*)0x1A55980;
        uint8_t* count = (uint8_t*)0xDA31C9;
        SuperSonicAnimationListSize = (*count + AnimationSetPatcher::m_newAnimationDataSuper.size());
        SuperSonicAnimationList = new CAnimationStateInfo[SuperSonicAnimationListSize];
        std::copy(pEntriesOriginal, pEntriesOriginal + *count, SuperSonicAnimationList);

        AnimationSetPatcher::initializeAnimationList(SuperSonicAnimationList, *count, AnimationSetPatcher::m_newAnimationDataSuper);
        WRITE_JUMP(0xDA31C8, UpdateSuperSonicAnimationListSize);
    }

    return pResult;
}

HOOK(void, __fastcall, CSonicSpRenderableSsnCreateAnimationStates, 0xDA31B0, void* This, void* Edx, void* A1, void* A2)
{
    originalCSonicSpRenderableSsnCreateAnimationStates(This, Edx, A1, A2);
    AnimationSetPatcher::createAnimationState(A2, AnimationSetPatcher::m_newAnimationDataSuper);
}

//---------------------------------------------------
// Static functions
//---------------------------------------------------
void AnimationSetPatcher::initializeAnimationList(CAnimationStateInfo* pEntries, size_t const count, NewAnimationDataList const& dataList)
{
    for (size_t i = 0; i < dataList.size(); i++)
    {
        NewAnimationData const& data = dataList[i];

        pEntries[count + i].m_Name = data.m_stateName;
        pEntries[count + i].m_FileName = data.m_fileName;
        pEntries[count + i].m_Speed = data.m_speed;
        pEntries[count + i].m_PlaybackType = !data.m_isLoop;
        pEntries[count + i].field10 = 0;
        pEntries[count + i].field14 = -1.0f;
        pEntries[count + i].field18 = -1.0f;
        pEntries[count + i].field1C = 0;
        pEntries[count + i].field20 = -1;
        pEntries[count + i].field24 = -1;
        pEntries[count + i].field28 = -1;
        pEntries[count + i].field2C = -1;
    }
}

void AnimationSetPatcher::createAnimationState(void* A2, NewAnimationDataList const& dataList)
{
    boost::shared_ptr<void> spAnimationState;
    for (NewAnimationData const& data : dataList)
    {
        fpCreateAnimationState(A2, spAnimationState, data.m_stateName, data.m_stateName);
    }

    // Set transition data
    for (NewAnimationData const& data : dataList)
    {
        if (!data.m_destinationState) continue;

        // Initialise data on destination state
        bool found = false;
        for (NewAnimationData const& destData : dataList)
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
            //MessageBox(NULL, L"Animation transition destination does not exist, please check your code!", NULL, MB_ICONERROR);
        }
    }
}

const char* volatile const AnimationSetPatcher::TrickSWA[7] =
{
    "TrickSWA_A",
    "TrickSWA_B",
    "TrickSWA_C",
    "TrickSWA_D",
    "TrickSWA_E",
    "TrickSWA_F",
    "TrickSWA_G"
};

const char* volatile const AnimationSetPatcher::TrickSG[7] =
{
    "TrickSG_A",
    "TrickSG_B",
    "TrickSG_C",
    "TrickSG_D",
    "TrickSG_E",
    "TrickSG_F",
    "TrickSG_G"
};

void AnimationSetPatcher::applyPatches()
{
    // DEBUG!!!
    //INSTALL_HOOK(CAnimationControlSingle_Debug);
    //INSTALL_HOOK(CSonicContextChangeAnimation);

    m_newAnimationData.emplace_back(TrickSWA[0], "sn_swa_trick_A", 1.0f, false, nullptr);
    m_newAnimationData.emplace_back(TrickSWA[1], "sn_swa_trick_B", 1.0f, false, nullptr);
    m_newAnimationData.emplace_back(TrickSWA[2], "sn_swa_trick_C", 1.0f, false, nullptr);
    m_newAnimationData.emplace_back(TrickSWA[3], "sn_swa_trick_D", 1.0f, false, nullptr);
    m_newAnimationData.emplace_back(TrickSWA[4], "sn_swa_trick_E", 1.0f, false, nullptr);
    m_newAnimationData.emplace_back(TrickSWA[5], "sn_swa_trick_F", 1.0f, false, nullptr);
    m_newAnimationData.emplace_back(TrickSWA[6], "sn_swa_trick_G", 1.0f, false, nullptr);

    m_newAnimationData.emplace_back(TrickSG[0], "sn_trick_A", 1.0f, false, nullptr);
    m_newAnimationData.emplace_back(TrickSG[1], "sn_trick_B", 1.0f, false, nullptr);
    m_newAnimationData.emplace_back(TrickSG[2], "sn_trick_C", 1.0f, false, nullptr);
    m_newAnimationData.emplace_back(TrickSG[3], "sn_trick_D", 1.0f, false, nullptr);
    m_newAnimationData.emplace_back(TrickSG[4], "sn_trick_E", 1.0f, false, nullptr);
    m_newAnimationData.emplace_back(TrickSG[5], "sn_trick_F", 1.0f, false, nullptr);
    m_newAnimationData.emplace_back(TrickSG[6], "sn_trick_G", 1.0f, false, nullptr);

    m_newAnimationDataSuper.emplace_back(TrickSWA[0], "ssn_trick_finish", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back(TrickSWA[1], "ssn_trick_finish", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back(TrickSWA[2], "ssn_trick_finish", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back(TrickSWA[3], "ssn_trick_finish", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back(TrickSWA[4], "ssn_trick_finish", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back(TrickSWA[5], "ssn_trick_finish", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back(TrickSWA[6], "ssn_trick_finish", 1.0f, false, nullptr);

    m_newAnimationDataSuper.emplace_back(TrickSG[0], "ssn_trick_finish", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back(TrickSG[1], "ssn_trick_finish", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back(TrickSG[2], "ssn_trick_finish", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back(TrickSG[3], "ssn_trick_finish", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back(TrickSG[4], "ssn_trick_finish", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back(TrickSG[5], "ssn_trick_finish", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back(TrickSG[6], "ssn_trick_finish", 1.0f, false, nullptr);

    if (!m_newAnimationData.empty())
    {
        INSTALL_HOOK(InitializeSonicAnimationList);
        INSTALL_HOOK(CSonicCreateAnimationStates);
    }

    if (!m_newAnimationDataSuper.empty())
    {
        INSTALL_HOOK(InitializeSuperSonicAnimationList);
        INSTALL_HOOK(CSonicSpRenderableSsnCreateAnimationStates);
    }
}
