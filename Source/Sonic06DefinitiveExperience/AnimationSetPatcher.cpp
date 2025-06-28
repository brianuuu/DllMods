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

HOOK(void, __fastcall, CSonicContextChangeAnimation, 0xE74CC0, Sonic::Player::CPlayerSpeedContext* context, int a2, Hedgehog::Base::CSharedString& name)
{
    printf("[AnimationManager] Animation change: %s\n", name.c_str());
    originalCSonicContextChangeAnimation(context, a2, name);
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

HOOK(void, __fastcall, CSonicCreateAnimationStates, 0xE1B6C0, void* This, void* Edx, Sonic::CAnimationStateMachine* A2, void* A3)
{
    AnimationSetPatcher::createAnimationState(A2, AnimationSetPatcher::m_newAnimationData);
    originalCSonicCreateAnimationStates(This, Edx, A2, A3);

    if (Configuration::m_model == Configuration::ModelType::Shadow)
    {
        A2->SetAnimationBlend("Glider", "GliderL", 0.2f);
        A2->SetAnimationBlend("Glider", "GliderR", 0.2f);
        A2->SetAnimationBlend("GliderL", "Glider", 0.2f);
        A2->SetAnimationBlend("GliderR", "Glider", 0.2f);
        A2->SetAnimationBlend("GliderR", "GliderL", 0.2f);
        A2->SetAnimationBlend("GliderL", "GliderR", 0.2f);

        A2->SetAnimationBlend("Hover", "HoverL", 0.1f);
        A2->SetAnimationBlend("Hover", "HoverR", 0.1f);
        A2->SetAnimationBlend("HoverL", "Hover", 0.1f);
        A2->SetAnimationBlend("HoverR", "Hover", 0.1f);
        A2->SetAnimationBlend("HoverR", "HoverL", 0.1f);
        A2->SetAnimationBlend("HoverL", "HoverR", 0.1f);
        A2->SetAnimationBlend("Hover", "HoverB", 0.1f);
        A2->SetAnimationBlend("HoverL", "HoverB", 0.1f);
        A2->SetAnimationBlend("HoverR", "HoverB", 0.1f);
        A2->SetAnimationBlend("HoverB", "Hover", 0.1f);
        A2->SetAnimationBlend("HoverBLoop", "Hover", 0.1f);
        A2->SetAnimationBlend("HoverBLoop", "HoverL", 0.1f);
        A2->SetAnimationBlend("HoverBLoop", "HoverR", 0.1f);

        A2->SetAnimationBlend("Bike", "BikeL", 0.1f);
        A2->SetAnimationBlend("Bike", "BikeR", 0.1f);
        A2->SetAnimationBlend("BikeL", "Bike", 0.1f);
        A2->SetAnimationBlend("BikeR", "Bike", 0.1f);
        A2->SetAnimationBlend("BikeR", "BikeL", 0.1f);
        A2->SetAnimationBlend("BikeL", "BikeR", 0.1f);
        A2->SetAnimationBlend("Bike", "BikeB", 0.1f);
        A2->SetAnimationBlend("BikeL", "BikeB", 0.1f);
        A2->SetAnimationBlend("BikeR", "BikeB", 0.1f);
        A2->SetAnimationBlend("BikeB", "Bike", 0.1f);
        A2->SetAnimationBlend("BikeBLoop", "Bike", 0.1f);
        A2->SetAnimationBlend("BikeBLoop", "BikeL", 0.1f);
        A2->SetAnimationBlend("BikeBLoop", "BikeR", 0.1f);
    }
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

const char* volatile const AnimationSetPatcher::AccelJumpLoop = "AccelJumpLoop";
const char* volatile const AnimationSetPatcher::FireTornadoLoop = "FireTornadoLoop";
const char* volatile const AnimationSetPatcher::FireTornadoEnd = "FireTornadoEnd";

const char* volatile const AnimationSetPatcher::GreenGemGround = "GreenGemGround";
const char* volatile const AnimationSetPatcher::GreenGemAir = "GreenGemAir";
const char* volatile const AnimationSetPatcher::SkyGem = "SkyGem";
const char* volatile const AnimationSetPatcher::SkyGemLoop = "SkyGemLoop";
const char* volatile const AnimationSetPatcher::SkyGemEnd = "SkyGemEnd";

const char* volatile const AnimationSetPatcher::FloatingBoost = "FloatingBoost";

const char* volatile const AnimationSetPatcher::SpinFast = "SpinFast";
const char* volatile const AnimationSetPatcher::ChaosAttack[5] = 
{ 
    "ChaosAttack00",
    "ChaosAttack01",
    "ChaosAttack02",
    "ChaosAttack03",
    "ChaosAttack04"
};
const char* volatile const AnimationSetPatcher::ChaosAttackWait = "ChaosAttackWait";
const char* volatile const AnimationSetPatcher::SpinAttack[3] =
{
    "SpinAttack00",
    "SpinAttack01",
    "SpinAttack02"
};
const char* volatile const AnimationSetPatcher::SpearWait = "SpearWait";
const char* volatile const AnimationSetPatcher::SpearWaitLoop = "SpearWaitLoop";
const char* volatile const AnimationSetPatcher::SpearShot = "SpearShot";
const char* volatile const AnimationSetPatcher::SpearShotLoop = "SpearShotLoop";
const char* volatile const AnimationSetPatcher::SpearSuperWait = "SpearSuperWait";
const char* volatile const AnimationSetPatcher::SpearSuperWaitLoop = "SpearSuperWaitLoop";
const char* volatile const AnimationSetPatcher::SpearSuperShot = "SpearSuperShot";
const char* volatile const AnimationSetPatcher::SpearSuperShotLoop = "SpearSuperShotLoop";
const char* volatile const AnimationSetPatcher::ChaosBoost = "ChaosBoost";
const char* volatile const AnimationSetPatcher::ChaosBoostLoop = "ChaosBoostLoop";
const char* volatile const AnimationSetPatcher::ChaosBlastWait = "ChaosBlastWait";
const char* volatile const AnimationSetPatcher::ChaosBlastWaitLoop = "ChaosBlastWaitLoop";
const char* volatile const AnimationSetPatcher::ChaosBlast = "ChaosBlast";
const char* volatile const AnimationSetPatcher::ChaosBlastLoop = "ChaosBlastLoop";

void AnimationSetPatcher::applyPatches()
{
    // DEBUG!!!
    //INSTALL_HOOK(CAnimationControlSingle_Debug);
    //INSTALL_HOOK(CSonicContextChangeAnimation);

    // Disable using blending animations since they cause crash
    INSTALL_HOOK(CSonic_AnimationBlending);
    WRITE_MEMORY(0x1278EFA, uint32_t, 0x15E7670); // sn_needle_blow_loop
    WRITE_MEMORY(0x1278F66, uint32_t, 0x15E7670); // sn_direct_l
    WRITE_MEMORY(0x1278FCD, uint32_t, 0x15E7670); // sn_direct_r

    // Fix ReactionPlate animations
    WRITE_MEMORY(0x1274C09, uint32_t, 0x15E7CF4); // sn_plate_h -> sn_airboost_loop   
    WRITE_MEMORY(0x1274C0E, uint8_t, 0x1D); // sn_plate_h -> loop
    WRITE_MEMORY(0x1274C70, uint32_t, 0x15E7CF4); // sn_plate_h -> sn_airboost_loop
    WRITE_MEMORY(0x1274C75, uint8_t, 0x1D); // sn_plate_h -> loop

    // Change Super Form dashring to use spring jump animation
    WRITE_MEMORY(0x1293D60, uint32_t, 0x15D5D8C); // DashRingL
    WRITE_MEMORY(0x1293DA7, uint32_t, 0x15D5D8C); // DashRingR

    // Set animations to loop
    WRITE_MEMORY(0x1276D20, uint8_t, 0x1D); // DashRingL
    WRITE_MEMORY(0x1276D87, uint8_t, 0x1D); // DashRingR

    // Fix Pulley
    WRITE_NOP(0xE468EE, 6); // disable PulleyJump waiting 10 frames
    WRITE_NOP(0xE469CB, 7); // allow SpinAttack animation after UpReel jump

    if (Configuration::m_usingSTH2006Project)
    {
        // Use idle animation on stage gates in STH2006 Project
        WRITE_MEMORY(0x127ADF1, uint32_t, 0x15E7670);
        WRITE_MEMORY(0x127AE58, uint32_t, 0x15E7670);
        WRITE_MEMORY(0x127AEAC, uint32_t, 0x15E7670);
        WRITE_MEMORY(0x127AF2A, uint32_t, 0x15E7670);
        WRITE_MEMORY(0x127AF91, uint32_t, 0x15E7670);
        WRITE_MEMORY(0x127AFF8, uint32_t, 0x15E7670);
    }

    // Add Super Sonic missing animation states
    // ssn_trick_jump
    // ssn_grind_loop
    // ssn_grind_switch
    // ssn_float_loop
    // ssn_idle_quickstep_l
    // ssn_idle_quickstep_r
    if (Configuration::m_model == Configuration::ModelType::Blaze || Configuration::m_model == Configuration::ModelType::Shadow)
    {
        // Grind Side Hops
        m_newAnimationDataSuper.emplace_back("GrindQuickJumpR", "sn_grind_jump_r", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindQuickJumpL", "sn_grind_jump_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindSideRightJumpR", "sn_grind_move_r_r", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindSideLeftJumpR", "sn_grind_move_l_r", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindSideRightJumpL", "sn_grind_move_r_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindSideLeftJumpL", "sn_grind_move_l_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindToWallROnRegular", "sn_grind_r_wall_r", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindToWallLOnRegular", "sn_grind_r_wall_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindToWallROnGoofy", "sn_grind_l_wall_r", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindToWallLOnGoofy", "sn_grind_l_wall_l", 1.0f, true, nullptr);

        // SkyDive
        m_newAnimationDataSuper.emplace_back("SkyDivingStart", "sn_dive_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("SkyDiving", "sn_dive_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("SkyDivingL", "sn_dive_l_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("SkyDivingR", "sn_dive_r_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("SkyDivingD", "sn_dive_fast_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("SkyDivingDEnd", "sn_dive_fast_e", 1.0f, false, nullptr);
        m_newAnimationDataSuper.emplace_back("SkyDivingDamage", "sn_dive_damage_loop", 1.0f, true, nullptr);

        // LightSpeedDash
        m_newAnimationDataSuper.emplace_back("LightSpeedDash", "sn_ringdash_loop", 1.0f, true, nullptr);
    }
    else
    {
        // Grind Side Hops
        m_newAnimationDataSuper.emplace_back("GrindQuickJumpR", "ssn_jump_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindQuickJumpL", "ssn_jump_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindSideRightJumpR", "ssn_jump_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindSideLeftJumpR", "ssn_jump_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindSideRightJumpL", "ssn_jump_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindSideLeftJumpL", "ssn_jump_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindToWallROnRegular", "ssn_jump_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindToWallLOnRegular", "ssn_jump_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindToWallROnGoofy", "ssn_jump_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GrindToWallLOnGoofy", "ssn_jump_loop", 1.0f, true, nullptr);

        // SkyDive
        m_newAnimationDataSuper.emplace_back("SkyDivingStart", "ssn_move_f_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("SkyDiving", "ssn_move_f_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("SkyDivingL", "ssn_move_f_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("SkyDivingR", "ssn_move_f_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("SkyDivingDEnd", "ssn_move_f_loop", 1.0f, true, nullptr);
    }
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

    // Fix Super Form animation
    WRITE_STRING(0x15D58F4, "ssn_trick_jump"); // TrickPrepare
    WRITE_STRING(0x15D5FD0, "ssn_float_loop"); // Float
    WRITE_STRING(0x15D59C8, "ssn_pulley_up_e"); // PulleyJump
    WRITE_STRING(0x15D5B70, "ssn_pulley_up_e"); // PoleSpinJumpStart
    if (Configuration::m_model == Configuration::ModelType::Blaze)
    {
        WRITE_STRING(0x15D5D04, "sn_spin_fall"); // JumpCloudStart
    }
    else
    {
        WRITE_STRING(0x15D5D04, "ssn_spin_fall"); // JumpCloudStart
    }

    m_newAnimationDataSuper.emplace_back("QuickStepL", "ssn_idle_quickstep_l", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back("QuickStepR", "ssn_idle_quickstep_r", 1.0f, false, nullptr);
    m_newAnimationDataSuper.emplace_back("HipSliding", "ssn_sliding_loop", 1.0f, true, nullptr);

    // SpinFall animation
    // sn_spin_fall
    // sn_spin_fall_spring
    // ssn_spin_fall
    // ssn_spin_fall_spring
    if (Configuration::m_model != Configuration::ModelType::SonicElise)
    {
        // Remove JumpBoard to Fall transition and add spin fall
        WRITE_JUMP(0xE1F503, (void*)0xE1F56E);
        m_newAnimationData.emplace_back(SpinFall, "sn_spin_fall", 1.0f, false, SpinFallLoop);
        m_newAnimationData.emplace_back(SpinFallSpring, "sn_spin_fall_spring", 1.0f, false, SpinFallLoop);
        m_newAnimationData.emplace_back(SpinFallLoop, "sn_jump_d_loop", 1.0f, true, nullptr);

        // Super Form
        if (Configuration::m_model == Configuration::ModelType::Blaze)
        {
            m_newAnimationDataSuper.emplace_back(SpinFall, "sn_spin_fall", 1.0f, false, SpinFallLoop);
            m_newAnimationDataSuper.emplace_back(SpinFallSpring, "sn_spin_fall_spring", 1.0f, false, SpinFallLoop);
            m_newAnimationDataSuper.emplace_back(SpinFallLoop, "sn_jump_d_loop", 1.0f, true, nullptr);
        }
        else
        {
            m_newAnimationDataSuper.emplace_back(SpinFall, "ssn_spin_fall", 1.0f, false, SpinFallLoop);
            m_newAnimationDataSuper.emplace_back(SpinFallSpring, "ssn_spin_fall_spring", 1.0f, false, SpinFallLoop);
            m_newAnimationDataSuper.emplace_back(SpinFallLoop, "ssn_jump_d_loop", 1.0f, true, nullptr);
        }

        // Set animations to loop
        WRITE_MEMORY(0x127779C, uint8_t, 0x1D); // UpReelEnd
        WRITE_MEMORY(0x1276B84, uint8_t, 0x1D); // JumpBoard
        WRITE_MEMORY(0x1276BEB, uint8_t, 0x1D); // JumpBoardRev
        WRITE_MEMORY(0x1276C4D, uint8_t, 0x1D); // JumpBoardSpecialL
        WRITE_MEMORY(0x1276CB9, uint8_t, 0x1D); // JumpBoardSpecialR
    }

    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        // Running goal
        if (Configuration::Sonic::m_run != Configuration::RunResultType::Disable)
        {
            m_newAnimationData.emplace_back(RunResult, "sn_result_run", 1.0f, false, RunResultLoop);
            m_newAnimationData.emplace_back(RunResultLoop, "sn_result_run_loop", 1.0f, true, nullptr);
        }

        // Brake flip (for 06 physics)
        m_newAnimationData.emplace_back(BrakeFlip, "sn_brake_flip", 1.0f, false, nullptr);

        // Squat Kick and Brake Flip for Super Sonic
        m_newAnimationDataSuper.emplace_back("SquatKick", "ssn_squat_kick", 1.0f, false, nullptr);
        m_newAnimationDataSuper.emplace_back(BrakeFlip, "ssn_brake_flip", 1.0f, false, nullptr);

        // Use same animation for homing attack
        WRITE_MEMORY(0x111832D + 6, uint32_t, 0x15F8F04);
        WRITE_MEMORY(0x1118337 + 6, uint32_t, 0x15F8F04);
        WRITE_MEMORY(0x1118341 + 6, uint32_t, 0x15F8F04);
        WRITE_MEMORY(0x111834B + 6, uint32_t, 0x15F8F04);
        WRITE_MEMORY(0x1118355 + 6, uint32_t, 0x15F8F04);

        // Green Gem
        m_newAnimationData.emplace_back(GreenGemGround, "sn_tornado_ground", 1.0f, false, nullptr);
        m_newAnimationData.emplace_back(GreenGemAir, "sn_tornado_air", 1.0f, false, nullptr);

        // Sky Gem
        m_newAnimationData.emplace_back(SkyGem, "sn_sky_gem_s", 1.0f, false, SkyGemLoop);
        m_newAnimationData.emplace_back(SkyGemLoop, "sn_sky_gem_loop", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back(SkyGemEnd, "sn_sky_gem_e", 1.0f, false, nullptr);
    }
    
    if (Configuration::m_model == Configuration::ModelType::SonicElise)
    {
        // Use unique animation for homing attack
        m_newAnimationData.emplace_back(HomingAttackLoop, "sn_homing_loop", 1.0f, true, nullptr);
    }

    if (Configuration::m_model == Configuration::ModelType::Blaze)
    {
        // Double Jump
        m_newAnimationData.emplace_back(AccelJumpLoop, "sn_accel_jump_loop", 2.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(AccelJumpLoop, "sn_accel_jump_loop", 2.0f, true, nullptr);

        // Fire Tornado
        m_newAnimationData.emplace_back(FireTornadoLoop, "sn_spin_attack_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(FireTornadoLoop, "sn_spin_attack_loop", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back(FireTornadoEnd, "sn_spin_attack_e", 1.0f, false, nullptr);
        m_newAnimationDataSuper.emplace_back(FireTornadoEnd, "sn_spin_attack_e", 1.0f, false, nullptr);
    
        // Floating Boost
        m_newAnimationData.emplace_back(FloatingBoost, "ssn_boost_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(FloatingBoost, "ssn_boost_loop", 1.0f, true, nullptr);
    }

    if (Configuration::m_model == Configuration::ModelType::Shadow)
    {
        // Float Boost
        if (Configuration::Shadow::m_floatBoost)
        {
            static char const* sn_boost_float_loop = "sn_boost_float_loop";
            WRITE_MEMORY(0x1272BE0, char*, sn_boost_float_loop);
            WRITE_MEMORY(0x1272C4F, char*, sn_boost_float_loop);
            WRITE_MEMORY(0x1272CBE, char*, sn_boost_float_loop);
            WRITE_MEMORY(0x1272D28, char*, sn_boost_float_loop);
            WRITE_MEMORY(0x1272D9C, char*, sn_boost_float_loop);
        }

        // Alt animation for Drifting
        m_newAnimationData.emplace_back(SpinFast, "sn_ball_loop", 1.5f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(SpinFast, "ssn_ball_loop", 1.5f, true, nullptr);

        // Brake flip (for 06 physics)
        m_newAnimationData.emplace_back(BrakeFlip, "sn_brake_flip", 1.0f, false, nullptr);

        // Squat Kick and Brake Flip for Super Shadow
        m_newAnimationDataSuper.emplace_back("SquatKick", "sn_squat_kick", 1.0f, false, nullptr);
        m_newAnimationDataSuper.emplace_back(BrakeFlip, "ssn_brake_flip", 1.0f, false, nullptr);

        // Floating Boost
        m_newAnimationData.emplace_back(FloatingBoost, "sn_boost_float_loop", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(FloatingBoost, "sn_boost_float_loop", 1.0f, true, nullptr);

        // Chaos Attack
        m_newAnimationData.emplace_back(ChaosAttack[0], "sh_chaos_attack00", 1.0f, false, ChaosAttackWait);
        m_newAnimationData.emplace_back(ChaosAttack[1], "sh_chaos_attack01", 1.0f, false, ChaosAttackWait);
        m_newAnimationData.emplace_back(ChaosAttack[2], "sh_chaos_attack02", 1.0f, false, ChaosAttackWait);
        m_newAnimationData.emplace_back(ChaosAttack[3], "sh_chaos_attack03", 1.0f, false, ChaosAttackWait);
        m_newAnimationData.emplace_back(ChaosAttack[4], "sh_chaos_attack04", 1.0f, false, ChaosAttackWait);
        m_newAnimationData.emplace_back(ChaosAttackWait, "sh_chaos_wait_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(ChaosAttack[0], "sh_chaos_attack00", 1.0f, false, ChaosAttackWait);
        m_newAnimationDataSuper.emplace_back(ChaosAttack[1], "sh_chaos_attack01", 1.0f, false, ChaosAttackWait);
        m_newAnimationDataSuper.emplace_back(ChaosAttack[2], "sh_chaos_attack02", 1.0f, false, ChaosAttackWait);
        m_newAnimationDataSuper.emplace_back(ChaosAttack[3], "sh_chaos_attack03", 1.0f, false, ChaosAttackWait);
        m_newAnimationDataSuper.emplace_back(ChaosAttack[4], "sh_chaos_attack04", 1.0f, false, ChaosAttackWait);
        m_newAnimationDataSuper.emplace_back(ChaosAttackWait, "sh_chaos_wait_l", 1.0f, true, nullptr);

        // Triple Kick
        m_newAnimationData.emplace_back(SpinAttack[0], "sh_spin_attack01_Root", 1.0f, false, nullptr);
        m_newAnimationData.emplace_back(SpinAttack[1], "sh_spin_attack02_Root", 1.0f, false, nullptr);
        m_newAnimationData.emplace_back(SpinAttack[2], "sh_spin_attack03_Root", 1.0f, false, nullptr);
        m_newAnimationDataSuper.emplace_back(SpinAttack[0], "sh_spin_attack01_Root", 1.0f, false, nullptr);
        m_newAnimationDataSuper.emplace_back(SpinAttack[1], "sh_spin_attack02_Root", 1.0f, false, nullptr);
        m_newAnimationDataSuper.emplace_back(SpinAttack[2], "ssh_spin_attack03_Root", 1.0f, false, nullptr);

        // Chaos Spear/Chaos Boost
        m_newAnimationData.emplace_back(SpearWait, "sh_spear_wait_s_Root", 1.0f, false, SpearWaitLoop);
        m_newAnimationData.emplace_back(SpearWaitLoop, "sh_spear_wait_l_Root", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back(SpearShot, "sh_spear_shot_s_Root", 1.0f, false, SpearShotLoop);
        m_newAnimationData.emplace_back(SpearShotLoop, "sh_spear_shot_l_Root", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back(SpearSuperWait, "ssh_chaos_store", 1.0f, false, SpearSuperWaitLoop);
        m_newAnimationData.emplace_back(SpearSuperWaitLoop, "ssh_chaos_store_l", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back(SpearSuperShot, "ssh_chaos_attack", 1.0f, false, SpearSuperShotLoop);
        m_newAnimationData.emplace_back(SpearSuperShotLoop, "ssh_chaos_attack_l", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back(ChaosBoost, "sh_spear_free_s_Root", 1.0f, false, ChaosBoostLoop);
        m_newAnimationData.emplace_back(ChaosBoostLoop, "sh_spear_free_l_Root", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(SpearWait, "sh_spear_wait_s_Root", 1.0f, false, SpearWaitLoop);
        m_newAnimationDataSuper.emplace_back(SpearWaitLoop, "sh_spear_wait_l_Root", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(SpearShot, "sh_spear_shot_s_Root", 1.0f, false, SpearShotLoop);
        m_newAnimationDataSuper.emplace_back(SpearShotLoop, "sh_spear_shot_l_Root", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(SpearSuperWait, "ssh_chaos_store", 1.0f, false, SpearSuperWaitLoop);
        m_newAnimationDataSuper.emplace_back(SpearSuperWaitLoop, "ssh_chaos_store_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(SpearSuperShot, "ssh_chaos_attack", 1.0f, false, SpearSuperShotLoop);
        m_newAnimationDataSuper.emplace_back(SpearSuperShotLoop, "ssh_chaos_attack_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(ChaosBoost, "sh_spear_free_s_Root", 1.0f, false, ChaosBoostLoop);
        m_newAnimationDataSuper.emplace_back(ChaosBoostLoop, "sh_spear_free_l_Root", 1.0f, true, nullptr);

        // Chaos Blast
        m_newAnimationData.emplace_back(ChaosBlastWait, "sh_chaosblast_s_Root", 1.0f, false, ChaosBlastWaitLoop);
        m_newAnimationData.emplace_back(ChaosBlastWaitLoop, "sh_chaosblast_wait_l_Root", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back(ChaosBlast, "sh_chaosblast_attack_s_Root", 1.0f, false, ChaosBlastLoop);
        m_newAnimationData.emplace_back(ChaosBlastLoop, "sh_chaosblast_attack_l_Root", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(ChaosBlastWait, "sh_chaosblast_s_Root", 1.0f, false, ChaosBlastWaitLoop);
        m_newAnimationDataSuper.emplace_back(ChaosBlastWaitLoop, "sh_chaosblast_wait_l_Root", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back(ChaosBlast, "sh_chaosblast_attack_s_Root", 1.0f, false, ChaosBlastLoop);
        m_newAnimationDataSuper.emplace_back(ChaosBlastLoop, "sh_chaosblast_attack_l_Root", 1.0f, true, nullptr);
    
        // Glider
        m_newAnimationData.emplace_back("Glider", "sh_gldr_ride_l", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back("GliderL", "sh_gldr_rideL_l_Root", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back("GliderR", "sh_gldr_rideR_l_Root", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("Glider", "sh_gldr_ride_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GliderL", "sh_gldr_rideL_l_Root", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("GliderR", "sh_gldr_rideR_l_Root", 1.0f, true, nullptr);

        // Hover
        m_newAnimationData.emplace_back("Hover", "sh_hover_ride_l", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back("HoverL", "sh_hover_curveL_l", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back("HoverR", "sh_hover_curveR_l", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back("HoverB", "sh_hover_back", 1.0f, false, "HoverBLoop");
        m_newAnimationData.emplace_back("HoverBLoop", "sh_hover_back_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("Hover", "sh_hover_ride_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("HoverL", "sh_hover_curveL_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("HoverR", "sh_hover_curveR_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("HoverB", "sh_hover_back", 1.0f, false, "HoverBLoop");
        m_newAnimationDataSuper.emplace_back("HoverBLoop", "sh_hover_back_l", 1.0f, true, nullptr);

        // Bike
        m_newAnimationData.emplace_back("Bike", "sh_bike_ride_l", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back("BikeL", "sh_bike_curveL_l", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back("BikeR", "sh_bike_curveR_l", 1.0f, true, nullptr);
        m_newAnimationData.emplace_back("BikeB", "sh_bike_back", 1.0f, false, "BikeBLoop");
        m_newAnimationData.emplace_back("BikeBLoop", "sh_bike_back_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("Bike", "sh_bike_ride_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("BikeL", "sh_bike_curveL_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("BikeR", "sh_bike_curveR_l", 1.0f, true, nullptr);
        m_newAnimationDataSuper.emplace_back("BikeB", "sh_bike_back", 1.0f, false, "BikeBLoop");
        m_newAnimationDataSuper.emplace_back("BikeBLoop", "sh_bike_back_l", 1.0f, true, nullptr);
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
