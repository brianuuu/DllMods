#include "AnimationSetPatcher.h"
#include "Configuration.h"

FUNCTION_PTR(void*, __stdcall, fpCreateAnimationState, 0xCDFA20, void* This, boost::shared_ptr<void>& spAnimationState, const Hedgehog::Base::CSharedString& name, const Hedgehog::Base::CSharedString& name2);
FUNCTION_PTR(uint32_t*, __stdcall, fpGetAnimationTransitionData, 0xCDFB40, void* A2, const Hedgehog::Base::CSharedString& name);

HOOK(bool, __fastcall, CAnimationControlSingle_Debug, 0x6D84F0, uint32_t** This, void* Edx, float a2, int a3)
{
    std::string name((char*)(This[58][2]));
    if (name.find("sn_") != std::string::npos)
    {
        printf("%s\n", name.c_str());
    }
    return originalCAnimationControlSingle_Debug(This, Edx, a2, a3);
}

HOOK(int*, __fastcall, CSonic_AnimationBlending, 0xE14A90, void* This, void* Edx, int a2, float a3)
{
    return nullptr;
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
    static uint32_t fnAddress = 0x6CBFC0;
    static uint32_t returnAddress = 0xDA31D4;
    __asm
    {
        push    SuperSonicAnimationListSize
        push    SuperSonicAnimationList
        call    [fnAddress]
        jmp     [returnAddress]
    }
}

std::vector<NewAnimationData> AnimationSetPatcher::m_newAnimationDataSuper;
HOOK(void*, __cdecl, InitializeSuperSonicAnimationList, 0x1291D60)
{
    void* pResult = originalInitializeSuperSonicAnimationList();
    {
        CAnimationStateInfo* pEntriesOriginal = *(CAnimationStateInfo**)0xDA31CB;
        uint32_t count = *(uint8_t*)0xDA31C9;

        // check if another DLL also has this function, get list from there
        if (*(uint8_t*)0xDA31C8 == 0xE9)
        {
            count = *(uint8_t*)0xDA31CD;
            pEntriesOriginal = *(CAnimationStateInfo**)0xDA31CE;
        }
        
        SuperSonicAnimationListSize = (count + AnimationSetPatcher::m_newAnimationDataSuper.size());
        SuperSonicAnimationList = new CAnimationStateInfo[SuperSonicAnimationListSize];
        std::copy(pEntriesOriginal, pEntriesOriginal + count, SuperSonicAnimationList);

        AnimationSetPatcher::initializeAnimationList(SuperSonicAnimationList, count, AnimationSetPatcher::m_newAnimationDataSuper);
        WRITE_JUMP(0xDA31C8, UpdateSuperSonicAnimationListSize);
        WRITE_MEMORY(0xDA31CD, uint8_t, (uint8_t)SuperSonicAnimationListSize);
        WRITE_MEMORY(0xDA31CE, void*, SuperSonicAnimationList);
    }

    return pResult;
}

HOOK(void, __fastcall, CSonicSpRenderableSsnCreateAnimationStates, 0xDA31B0, void* This, void* Edx, void* A1, void* A2)
{
    originalCSonicSpRenderableSsnCreateAnimationStates(This, Edx, A1, A2);
    AnimationSetPatcher::createAnimationState(A2, AnimationSetPatcher::m_newAnimationDataSuper);
}

//---------------------------------------------------
// CSonicClassic
//---------------------------------------------------
std::vector<NewAnimationData> AnimationSetPatcher::m_newAnimationClassicData;
HOOK(void*, __cdecl, InitializeClassicSonicAnimationList, 0x1281D50)
{
    void* pResult = originalInitializeClassicSonicAnimationList();
    {
        CAnimationStateSet* pList = (CAnimationStateSet*)0x15DCE60;
        size_t newCount = pList->m_Count + AnimationSetPatcher::m_newAnimationClassicData.size();
        CAnimationStateInfo* pEntries = new CAnimationStateInfo[newCount];
        std::copy(pList->m_pEntries, pList->m_pEntries + pList->m_Count, pEntries);

        AnimationSetPatcher::initializeAnimationList(pEntries, pList->m_Count, AnimationSetPatcher::m_newAnimationClassicData);
        WRITE_MEMORY(&pList->m_pEntries, void*, pEntries);
        WRITE_MEMORY(&pList->m_Count, size_t, newCount);
    }

    return pResult;
}

HOOK(void, __fastcall, CSonicClassicCreateAnimationStates, 0xDDF1C0, void* This, void* Edx, void* A2, void* A3)
{
    AnimationSetPatcher::createAnimationState(A2, AnimationSetPatcher::m_newAnimationClassicData);
    originalCSonicClassicCreateAnimationStates(This, Edx, A2, A3);
}

//---------------------------------------------------
// CSonicSpRenderableSsc
//---------------------------------------------------
CAnimationStateInfo* ClassicSuperSonicAnimationList = nullptr;
uint32_t ClassicSuperSonicAnimationListSize = 0;
void __declspec(naked) UpdateClassicSuperSonicAnimationListSize()
{
    static uint32_t returnAddress = 0xDA8FC2;
    __asm
    {
        push    ClassicSuperSonicAnimationListSize
        push    ClassicSuperSonicAnimationList
        jmp     [returnAddress]
    }
}

std::vector<NewAnimationData> AnimationSetPatcher::m_newAnimationClassicDataSuper;
HOOK(void*, __cdecl, InitializeClassicSuperSonicAnimationList, 0x128F600)
{
    void* pResult = originalInitializeClassicSuperSonicAnimationList();
    {
        CAnimationStateInfo* pEntriesOriginal = (CAnimationStateInfo*)0x1A54660;
        uint8_t* count = (uint8_t*)0xDA8FBC;
        ClassicSuperSonicAnimationListSize = (*count + AnimationSetPatcher::m_newAnimationClassicDataSuper.size());
        ClassicSuperSonicAnimationList = new CAnimationStateInfo[ClassicSuperSonicAnimationListSize];
        std::copy(pEntriesOriginal, pEntriesOriginal + *count, ClassicSuperSonicAnimationList);

        AnimationSetPatcher::initializeAnimationList(ClassicSuperSonicAnimationList, *count, AnimationSetPatcher::m_newAnimationClassicDataSuper);
        WRITE_JUMP(0xDA8FBB, UpdateClassicSuperSonicAnimationListSize);
    }

    return pResult;
}

HOOK(void, __fastcall, CSonicSpRenderableSscCreateAnimationStates, 0xDA8F90, void* This, void* Edx, void* A1, void* A2)
{
    originalCSonicSpRenderableSscCreateAnimationStates(This, Edx, A1, A2);
    AnimationSetPatcher::createAnimationState(A2, AnimationSetPatcher::m_newAnimationClassicDataSuper);
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
            MessageBox(NULL, L"Animation transition destination does not exist, please check your code!", NULL, MB_ICONERROR);
        }
    }
}

const char* volatile const AnimationSetPatcher::FlyLoop = "FlyLoop";
const char* volatile const AnimationSetPatcher::FlyTired = "FlyTired";
const char* volatile const AnimationSetPatcher::FlyTiredLoop = "FlyTiredLoop";
const char* volatile const AnimationSetPatcher::SwimLoop = "SwimLoop";
const char* volatile const AnimationSetPatcher::SwimTired = "SwimTired";
const char* volatile const AnimationSetPatcher::SwimTiredLoop = "SwimTiredLoop";

void AnimationSetPatcher::applyPatches()
{
    // DEBUG!!!
    //INSTALL_HOOK(CAnimationControlSingle_Debug);
    //INSTALL_HOOK(CSonicContextChangeAnimation);

    m_newAnimationClassicData.emplace_back(FlyLoop, "tc_fly_loop", 1.0f, true, nullptr);
    m_newAnimationClassicData.emplace_back(FlyTired, "tc_fly_tired_s", 1.0f, false, FlyTiredLoop);
    m_newAnimationClassicData.emplace_back(FlyTiredLoop, "tc_fly_tired_loop", 1.0f, true, nullptr);

    m_newAnimationClassicData.emplace_back(SwimLoop, "tc_swim_loop", 1.0f, true, nullptr);
    m_newAnimationClassicData.emplace_back(SwimTired, "tc_swim_tired_s", 1.0f, false, SwimTiredLoop);
    m_newAnimationClassicData.emplace_back(SwimTiredLoop, "tc_swim_tired_loop", 1.0f, true, nullptr);

    m_newAnimationClassicDataSuper.emplace_back(FlyLoop, "stc_fly_loop", 1.0f, true, nullptr);
    m_newAnimationClassicDataSuper.emplace_back(FlyTired, "stc_fly_tired_s", 1.0f, false, FlyTiredLoop);
    m_newAnimationClassicDataSuper.emplace_back(FlyTiredLoop, "stc_fly_tired_loop", 1.0f, true, nullptr);

    m_newAnimationClassicDataSuper.emplace_back(SwimLoop, "stc_swim_loop", 1.0f, true, nullptr);
    m_newAnimationClassicDataSuper.emplace_back(SwimTired, "stc_swim_tired_s", 1.0f, false, SwimTiredLoop);
    m_newAnimationClassicDataSuper.emplace_back(SwimTiredLoop, "stc_swim_tired_loop", 1.0f, true, nullptr);

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

    if (!m_newAnimationClassicData.empty())
    {
        INSTALL_HOOK(InitializeClassicSonicAnimationList);
        INSTALL_HOOK(CSonicClassicCreateAnimationStates);
    }

    if (!m_newAnimationClassicDataSuper.empty())
    {
        INSTALL_HOOK(InitializeClassicSuperSonicAnimationList);
        INSTALL_HOOK(CSonicSpRenderableSscCreateAnimationStates);
    }
}
