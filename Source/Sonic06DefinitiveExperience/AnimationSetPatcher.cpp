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

HOOK(int*, __fastcall, CSonic_AnimationBlending, 0xE14A90, void* This, void* Edx, int a2, float a3)
{
    return nullptr;
}

HOOK(void, __stdcall, CSonicContextChangeAnimation, 0xCDFC80, void* This, int a2, Hedgehog::Base::CSharedString& name)
{
    printf("[AnimationManager] Animation change: %s\n", name.m_pStr);
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
            MessageBox(NULL, L"Animation transition destination does not exist, please check your code!", NULL, MB_ICONERROR);
        }
    }
}

const char* volatile const AnimationSetPatcher::RunResult = "RunResult";
const char* volatile const AnimationSetPatcher::RunResultLoop = "RunResultLoop";
const char* volatile const AnimationSetPatcher::BrakeFlip = "BrakeFlip";
const char* volatile const AnimationSetPatcher::SpinFall = "SpinFall";
const char* volatile const AnimationSetPatcher::SpinFallSpring = "SpinFallSpring";
const char* volatile const AnimationSetPatcher::SpinFallLoop = "SpinFallLoop";
const char* volatile const AnimationSetPatcher::HomingAttackLoop = "HomingAttackLoop";

void AnimationSetPatcher::applyPatches()
{
    // DEBUG!!!
    //INSTALL_HOOK(CAnimationControlSingle_Debug);
    //INSTALL_HOOK(CSonicContextChangeAnimation);

    // Disable using blending animations since they cause crash
    INSTALL_HOOK(CSonic_AnimationBlending);
    WRITE_MEMORY(0x1274A6D, uint32_t, 0x15E7670); // sn_plate_v_l
    WRITE_MEMORY(0x1274AD4, uint32_t, 0x15E7670); // sn_plate_v_l
    WRITE_MEMORY(0x1274B3B, uint32_t, 0x15E7670); // sn_plate_v_r
    WRITE_MEMORY(0x1274BA2, uint32_t, 0x15E7670); // sn_plate_v_r
    WRITE_MEMORY(0x1274C09, uint32_t, 0x15E7670); // sn_plate_h
    WRITE_MEMORY(0x1274C70, uint32_t, 0x15E7670); // sn_plate_h
    WRITE_MEMORY(0x1278EFA, uint32_t, 0x15E7670); // sn_needle_blow_loop
    WRITE_MEMORY(0x1278F66, uint32_t, 0x15E7670); // sn_direct_l
    WRITE_MEMORY(0x1278FCD, uint32_t, 0x15E7670); // sn_direct_r

    // Change Super Form dashring to use spring jump animation
    WRITE_MEMORY(0x1293D60, uint32_t, 0x15D5D8C); // DashRingL
    WRITE_MEMORY(0x1293DA7, uint32_t, 0x15D5D8C); // DashRingR

    // Set animations to loop
    WRITE_MEMORY(0x1276D20, uint8_t, 0x1D); // DashRingL
    WRITE_MEMORY(0x1276D87, uint8_t, 0x1D); // DashRingR

    // Add Super Sonic missing animation states
    // ssn_trick_jump
    // ssn_grind_loop
    // ssn_grind_switch
    // ssn_float_loop
    m_newAnimationDataSuper.emplace_back("SkyDivingStart", "ssn_move_f_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("SkyDiving", "ssn_move_f_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("SkyDivingL", "ssn_move_f_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("SkyDivingR", "ssn_move_f_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("SkyDivingDEnd", "ssn_move_f_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindQuickJumpR", "ssn_jump_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindQuickJumpL", "ssn_jump_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindSideRightJumpR", "ssn_jump_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindSideLeftJumpR", "ssn_jump_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindSideRightJumpL", "ssn_jump_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindSideLeftJumpL", "ssn_jump_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindLandR", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindLandL", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindStandL", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindStandR", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindStandLongL", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindStandLongR", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindStandRTurnR", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindStandRTurnL", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindStandLTurnR", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindStandLTurnL", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindStandFrontR", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindStandFrontL", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindStandBackR", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindStandBackL", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindSurpriseR", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindSurpriseL", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindSquatL", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindSquatR", "ssn_grind_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindSwitchAL", "ssn_grind_switch", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindSwitchAR", "ssn_grind_switch", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindToWallROnRegular", "ssn_jump_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindToWallLOnRegular", "ssn_jump_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindToWallROnGoofy", "ssn_jump_loop", 1.0f, true, nullptr);
    m_newAnimationDataSuper.emplace_back("GrindToWallLOnGoofy", "ssn_jump_loop", 1.0f, true, nullptr);

    // Fix Super Form animation
    WRITE_STRING(0x15D58F4, "ssn_trick_jump"); // TrickPrepare
    WRITE_STRING(0x15D5FD0, "ssn_float_loop"); // Float

    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        // Running goal
        if (Configuration::m_run != Configuration::RunResultType::Disable)
        {
            m_newAnimationData.emplace_back(RunResult, "sn_result_run", 1.0f, false, RunResultLoop);
            m_newAnimationData.emplace_back(RunResultLoop, "sn_result_run_loop", 1.0f, true, nullptr);
        }

        // Brake flip (for 06 physics)
        m_newAnimationData.emplace_back(BrakeFlip, "sn_brake_flip", 1.0f, false, nullptr);

        // Squat Kick and Brake Flip for Super Sonic
        m_newAnimationDataSuper.emplace_back("SquatKick", "ssn_squat_kick", 1.0f, false, nullptr);
        m_newAnimationDataSuper.emplace_back(BrakeFlip, "ssn_brake_flip", 1.0f, false, nullptr);

        // Remove JumpBoard to Fall transition and add spin fall
        WRITE_JUMP(0xE1F503, (void*)0xE1F56E);
        m_newAnimationData.emplace_back(SpinFall, "sn_spin_fall", 1.0f, false, SpinFallLoop);
        m_newAnimationData.emplace_back(SpinFallSpring, "sn_spin_fall_spring", 1.0f, false, SpinFallLoop);
        m_newAnimationData.emplace_back(SpinFallLoop, "sn_jump_d_loop", 1.0f, true, nullptr);

        // Set animations to loop
        WRITE_MEMORY(0x127779C, uint8_t, 0x1D); // UpReelEnd
        WRITE_MEMORY(0x1276B84, uint8_t, 0x1D); // JumpBoard
        WRITE_MEMORY(0x1276BEB, uint8_t, 0x1D); // JumpBoardRev
        WRITE_MEMORY(0x1276C4D, uint8_t, 0x1D); // JumpBoardSpecialL
        WRITE_MEMORY(0x1276CB9, uint8_t, 0x1D); // JumpBoardSpecialR
    }
    
    if (Configuration::m_model == Configuration::ModelType::SonicElise)
    {
        // Use unique animation for homing attack
        m_newAnimationData.emplace_back(HomingAttackLoop, "sn_homing_loop", 1.0f, true, nullptr);
    }

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
