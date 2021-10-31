#include "NextGenPhysics.h"
#include "Configuration.h"

#include "NextGenSonic.h"

float const NextGenPhysics::c_funcMaxTurnRate = 400.0f;
float const NextGenPhysics::c_funcTurnRateMultiplier = PI_F * 10.0f;
float NextGenPhysics::m_homingDownSpeed = 0.0f;
float const c_homingDownSpeedAdd = 15.0f;

HOOK(void, __stdcall, NextGenSonic_CSonicRotationAdvance, 0xE310A0, void* a1, float* targetDir, float turnRate1, float turnRateMultiplier, bool noLockDirection, float turnRate2)
{
    CSonicStateFlags* flags = Common::GetSonicStateFlags();
    if (flags->KeepRunning || flags->Boost)
    {
        // In auto-run section, reduce turning rate so player can tap joystick and not get flinged
        Eigen::Vector3f playerVelocity;
        Common::GetPlayerVelocity(playerVelocity);

        // 100 turn rate at speed 60, 90 turn rate at speed 85
        float speed = playerVelocity.norm();
        float turnRate = 0.4f * (85.0f - speed) + 90.0f;
        Common::ClampFloat(turnRate, 80.0f, flags->KeepRunning ? 100.0f : 110.0f);

        originalNextGenSonic_CSonicRotationAdvance(a1, targetDir, turnRate, NextGenPhysics::c_funcTurnRateMultiplier * 0.5f, noLockDirection, turnRate);
    }
    else if (noLockDirection && !Common::IsPlayerOnBoard())
    {
        // If direction is not locked, pump up turn rate
        originalNextGenSonic_CSonicRotationAdvance(a1, targetDir, NextGenPhysics::c_funcMaxTurnRate, NextGenPhysics::c_funcTurnRateMultiplier, noLockDirection, NextGenPhysics::c_funcMaxTurnRate);
    }
    else
    {
        originalNextGenSonic_CSonicRotationAdvance(a1, targetDir, turnRate1, turnRateMultiplier, noLockDirection, turnRate2);
    }
}

void NextGenPhysics::applyCSonicRotationAdvance(void* This, float* targetDir, float turnRate1, float turnRateMultiplier, bool noLockDirection, float turnRate2)
{
    originalNextGenSonic_CSonicRotationAdvance(This, targetDir, turnRate1, turnRateMultiplier, noLockDirection, turnRate2);
}

bool NextGenPhysics::checkUseLightSpeedDash()
{
    if (!*PLAYER_CONTEXT) return false;
    if (*(uint32_t*)((uint32_t)*PLAYER_CONTEXT + 0x12BC))
    {
        *(uint32_t*)((uint32_t)*PLAYER_CONTEXT + 0x12C0) = 0;
        StateManager::ChangeState(StateAction::LightSpeedDash, *PLAYER_CONTEXT);
        return true;
    }

    return false;
}

HOOK(void, __fastcall, NextGenPhysics_CSonicSetMaxSpeedBasis, 0xDFBCA0, int* This)
{
    originalNextGenPhysics_CSonicSetMaxSpeedBasis(This);

    if (Common::IsPlayerOnBoard() && !Common::GetSonicStateFlags()->Boost)
    {
        *Common::GetPlayerMaxSpeed() = 26.0f;
    }
}

HOOK(void, __fastcall, NextGenPhysics_MsgApplyImpulse, 0xE6CFA0, void* This, void* Edx, MsgApplyImpulse* message)
{
    // Fix trick start animation
    WRITE_MEMORY(0xE6D3E8, uint32_t, 0x15F8CC4); // TrickPrepare

    if (Common::IsPlayerSuper())
    {
        // Fix Super Form JumpBoard animation
        WRITE_MEMORY(0xE6D1C2, uint32_t, 0x15F8A04); // JumpBoard
        WRITE_MEMORY(0xE6D23F, uint32_t, 0x15F8A04); // JumpBoardSpecialL
        WRITE_MEMORY(0xE6D209, uint32_t, 0x15F8A04); // JumpBoardSpecialR
    }
    else
    {
        // Original code
        WRITE_MEMORY(0xE6D1C2, uint32_t, 0x15F89C0); // JumpBoard
        WRITE_MEMORY(0xE6D23F, uint32_t, 0x15F89DC); // JumpBoardSpecialL
        WRITE_MEMORY(0xE6D209, uint32_t, 0x15F89F0); // JumpBoardSpecialR
    }

    originalNextGenPhysics_MsgApplyImpulse(This, Edx, message);
}

SharedPtrTypeless homingPfxHandle;
HOOK(int, __fastcall, NextGenPhysics_CSonicStateHomingAttackBegin, 0x1232040, void* This)
{
    // Remember down speed just before homing attack
    if (Configuration::m_physics)
    {
        Eigen::Vector3f playerVelocity;
        Common::GetPlayerVelocity(playerVelocity);
        NextGenPhysics::m_homingDownSpeed = min(playerVelocity.y(), 0.0f);
        //printf("Down speed = %.3f\n", NextGenPhysics::m_homingDownSpeed);
    }

    // Play homing attack pfx
    void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
    Common::fCGlitterCreate(*PLAYER_CONTEXT, homingPfxHandle, matrixNode, "ef_ch_sng_homing", 1);

    return originalNextGenPhysics_CSonicStateHomingAttackBegin(This);
}

HOOK(int*, __fastcall, NextGenPhysics_CSonicStateHomingAttackEnd, 0x1231F80, void* This)
{
    // Apply down speed before homing attack
    if (Configuration::m_physics && StateManager::isCurrentAction(StateAction::Fall))
    {
        Eigen::Vector3f playerVelocity;
        Common::GetPlayerVelocity(playerVelocity);
        playerVelocity.y() += NextGenPhysics::m_homingDownSpeed - c_homingDownSpeedAdd;
        Common::SetPlayerVelocity(playerVelocity);
    }

    // Kill homing pfx
    Common::fCGlitterEnd(*PLAYER_CONTEXT, homingPfxHandle, false);

    return originalNextGenPhysics_CSonicStateHomingAttackEnd(This);
}

void __declspec(naked) noAirDashOutOfControl()
{
    static uint32_t returnAddress = 0x1232445;
    static uint32_t skipAddress = 0x1232450;
    __asm
    {
        mov     byte ptr [ecx + 17h], 0

        // Check where we have lock-on target
        cmp     dword ptr[ebx + 0E98h], 0
        jnz     jump

        // Air dash, skip out of control
        mov     [esi + 80h], 0
        jmp     [skipAddress]

        // Out of control prep
        jump:
        push    ecx
        fstp    [esp]
        push    ebx
        jmp     [returnAddress]
    }
}

void __declspec(naked) lightDashHigherPriority()
{
    static uint32_t returnAddress = 0xDFDDD0;
    static uint32_t successAddress = 0xDFDDED;
    static uint32_t fpLightSpeedDash = 0xDFB3F0;
    __asm
    {
        // Check light speed dash
        call    [fpLightSpeedDash]
        test    al, al
        jz      jump
        jmp     [successAddress]

        // Original check B-button actions
        jump:
        push    [0x15F55A4]
        jmp     [returnAddress]
    }
}

void __declspec(naked) noTrickRainbowRing()
{
    static uint32_t returnAddress = 0xE6D417;
    __asm
    {
        // Copied from dash ring case 8
        mov     eax, [ebx]
        mov     edx, [eax + 11Ch]
        push    1
        push    1
        push    ecx
        lea     ecx, [esp + 0ACh - 64h]
        fstp    [esp]
        push    ecx
        mov     ecx, ebx
        call    edx

        jmp     [returnAddress]
    }
}

void __declspec(naked) CObjPlaTramCarBoostButtonChange()
{
    static uint32_t successAddress = 0xF368C4;
    static uint32_t returnAddress = 0xF3691D;
    __asm
    {
        test    word ptr [esi], 0x8000
        jz      jump
        jmp     [successAddress]

        jump:
        jmp     [returnAddress]
    }
}

float m_lightdashCountdown = 0.0f;
HOOK(int*, __fastcall, NextGenPhysics_CSonicStateLightSpeedDashAdvance, 0x1231810, void* This)
{
    if (m_lightdashCountdown < 1.5f && Common::GetSonicStateFlags()->KeepRunning)
    {
        WRITE_JUMP(0xE16CCE, (void*)0xE16EEB);
        m_lightdashCountdown = 1.5f;
    }
    return originalNextGenPhysics_CSonicStateLightSpeedDashAdvance(This);
}

HOOK(void, __fastcall, NextGenPhysics_CSonicUpdate, 0xE6BF20, void* This, void* Edx, float* dt)
{
    if (m_lightdashCountdown > 0.0f)
    {
        m_lightdashCountdown -= *dt;
        if (m_lightdashCountdown <= 0.0f)
        {
            WRITE_MEMORY(0xE16CCE, uint8_t, 0x0F, 0x85, 0x17, 0x02, 0x00, 0x00);
        }
    }
    originalNextGenPhysics_CSonicUpdate(This, Edx, dt);
}

void NextGenPhysics::applyPatches()
{
    // Change character movement aniamtion speeds
    applyCharacterAnimationSpeed();

    // Always disable jump ball sfx
    WRITE_MEMORY(0x11BCC7E, int, -1);

    // Always disable lightdash voice
    WRITE_MEMORY(0x1231964, uint8_t, 0xEB);

    // Fix Super Form's JumpBoard animation, and fix trick animation
    INSTALL_HOOK(NextGenPhysics_MsgApplyImpulse);

    // Maintain down speed when homing attack finished (for 06 physics)
    INSTALL_HOOK(NextGenPhysics_CSonicStateHomingAttackBegin);
    INSTALL_HOOK(NextGenPhysics_CSonicStateHomingAttackEnd);

    // Disable trick system
    if (Configuration::m_noTrick)
    {
        // No trick rainbow ring, but keep rainbow ring animation
        WRITE_JUMP(0xE6D3FB, noTrickRainbowRing);

        // Make trick ramp use JumpBaord animation
        WRITE_MEMORY(0x1014866, uint32_t, ImpulseType::JumpBoard);
    }

    // 06 physics general code
    if (Configuration::m_physics)
    {
        // Increase turning rate
        INSTALL_HOOK(NextGenSonic_CSonicRotationAdvance);

        // No out of control for air dash
        WRITE_JUMP(0x123243C, noAirDashOutOfControl);

        // Disable jumpball collision
        WRITE_MEMORY(0x1118FFA, uint8_t, 0xC2, 0xC5); // Fall state
        WRITE_MEMORY(0x11BCC4B, uint8_t, 0x71); // JumpBall

        // Drop all rings when getting damaged
        WRITE_MEMORY(0xE6628E, uint8_t, 0xEB);
        WRITE_MEMORY(0xE6CCDE, uint8_t, 0xEB);

        // Make board speed slightly faster
        INSTALL_HOOK(NextGenPhysics_CSonicSetMaxSpeedBasis);
    }

    // Change all actions to X button, change boost to R2
    if (Configuration::m_xButtonAction)
    {
        // Check for light speed dash before stomp
        WRITE_JUMP(0xDFDDCB, lightDashHigherPriority);

        // X actions
        WRITE_MEMORY(0xDFDDBD, uint32_t, 4);    // Stomping
        WRITE_MEMORY(0xDFDE7F, uint32_t, 4);    // Unknown B hold
        WRITE_MEMORY(0xDFDEBA, uint32_t, 4);    // GrindSquat start
        WRITE_MEMORY(0xDFF6E5, uint32_t, 4);    // Stomp, slide, squat start
        WRITE_MEMORY(0xDFFD5F, uint32_t, 4);    // LightSpeedDashReady
        WRITE_MEMORY(0xE4910E, uint8_t, 4);     // Board Squat
        WRITE_MEMORY(0x111761A, uint32_t, 4);   // LightSpeedDash
        WRITE_MEMORY(0x1118970, uint32_t, 4);   // GrindSquat end
        WRITE_MEMORY(0x11D6B03, uint32_t, 4);   // Sliding end
        WRITE_MEMORY(0x1230BB6, uint32_t, 4);   // Squat end

        // R2 actions
        WRITE_MEMORY(0xDFF25B, uint32_t, 32);   // Grind Boost
        WRITE_MEMORY(0xDFDF4C, uint32_t, 32);   // Air Boost
        WRITE_MEMORY(0xE3D991, uint32_t, 32);   // Blast Off
        WRITE_MEMORY(0xE4776B, uint32_t, 32);   // Dummy Boost
        WRITE_MEMORY(0x11177EE, uint32_t, 32);  // Boost
        WRITE_MEMORY(0x1118CEE, uint32_t, 32);  // Fall Boost
        WRITE_MEMORY(0x111BE61, uint32_t, 32);  // Null Boost?
        WRITE_MEMORY(0x111BEE8, uint32_t, 32);  // Dummy Boost plugin
        WRITE_MEMORY(0x111D801, uint32_t, 32);  // Board Fall Boost
        WRITE_MEMORY(0x11A0716, uint32_t, 32);  // Dummy Boost External
        WRITE_MEMORY(0x11A07D7, uint32_t, 32);  // Speed external control
        WRITE_MEMORY(0x11A0DA8, uint32_t, 32);  // Speed external control
        WRITE_MEMORY(0x11BD057, uint32_t, 32);  // DivingDive start
        WRITE_MEMORY(0x124AF01, uint32_t, 32);  // DivingDive end
        WRITE_JUMP(0xF368BF, CObjPlaTramCarBoostButtonChange); // PlaTramCar

        // Map drift to B-button
        WRITE_MEMORY(0xDF2DFF, uint32_t, 1);
        WRITE_MEMORY(0xDFF62B, uint32_t, 1);
        WRITE_NOP(0xDFF80D, 2);
        WRITE_MEMORY(0xDFF810, uint8_t, 0xB3);
        WRITE_MEMORY(0xDFF816, uint32_t, 1);
        WRITE_MEMORY(0xDFF81B, uint8_t, 0x81);
        WRITE_MEMORY(0x1119549, uint32_t, 1);
        WRITE_MEMORY(0x119910D, uint32_t, 1);

        // TODO: Change notification buttons?
    }

    // After light dash, force using mach speed animation
    INSTALL_HOOK(NextGenPhysics_CSonicStateLightSpeedDashAdvance);
    INSTALL_HOOK(NextGenPhysics_CSonicUpdate);

    // Apply character specific patches
    switch (Configuration::m_model)
    {
    case Configuration::ModelType::Sonic:
    case Configuration::ModelType::SonicElise:
    {
        NextGenSonic::applyPatches();
        break;
    }
    }
}

NextGenAnimation NextGenPhysics::m_animationData = NextGenAnimation();
void NextGenPhysics::applyCharacterAnimationSpeed()
{
    // Modifying movement animation speed
    switch (Configuration::m_model)
    {
        case Configuration::ModelType::Sonic:
        {
            NextGenSonic::setAnimationSpeed_Sonic(m_animationData);
            break;
        }
        case Configuration::ModelType::SonicElise:
        {
            NextGenSonic::setAnimationSpeed_Elise(m_animationData);
            break;
        }
    }

    WRITE_MEMORY(0x12724AA, float*, &m_animationData.walk_playbackSpeed);
    WRITE_MEMORY(0x12724D7, float*, &m_animationData.walk_speedFactor);

    WRITE_MEMORY(0x127251B, float*, &m_animationData.walkFast_playbackSpeed);
    WRITE_MEMORY(0x1272546, float*, &m_animationData.walkFast_speedFactor);

    WRITE_MEMORY(0x127258A, float*, &m_animationData.jog_playbackSpeed);
    WRITE_MEMORY(0x12725F9, float*, &m_animationData.jog_playbackSpeed);
    WRITE_MEMORY(0x1272668, float*, &m_animationData.jog_playbackSpeed);
    WRITE_MEMORY(0x12725B5, float*, &m_animationData.jog_speedFactor);
    WRITE_MEMORY(0x1272624, float*, &m_animationData.jog_speedFactor);
    WRITE_MEMORY(0x1272693, float*, &m_animationData.jog_speedFactor);

    WRITE_MEMORY(0x12726D7, float*, &m_animationData.run_playbackSpeed);
    WRITE_MEMORY(0x1272746, float*, &m_animationData.run_playbackSpeed);
    WRITE_MEMORY(0x12727B5, float*, &m_animationData.run_playbackSpeed);
    WRITE_MEMORY(0x1272702, float*, &m_animationData.run_speedFactor);
    WRITE_MEMORY(0x1272771, float*, &m_animationData.run_speedFactor);
    WRITE_MEMORY(0x12727E0, float*, &m_animationData.run_speedFactor);

    WRITE_MEMORY(0x1272824, float*, &m_animationData.dash_playbackSpeed);
    WRITE_MEMORY(0x1272893, float*, &m_animationData.dash_playbackSpeed);
    WRITE_MEMORY(0x1272902, float*, &m_animationData.dash_playbackSpeed);
    WRITE_MEMORY(0x127284F, float*, &m_animationData.dash_speedFactor);
    WRITE_MEMORY(0x12728BE, float*, &m_animationData.dash_speedFactor);
    WRITE_MEMORY(0x127292D, float*, &m_animationData.dash_speedFactor);

    WRITE_MEMORY(0x1272971, float*, &m_animationData.jet_playbackSpeed);
    WRITE_MEMORY(0x12729E0, float*, &m_animationData.jet_playbackSpeed);
    WRITE_MEMORY(0x1272A4F, float*, &m_animationData.jet_playbackSpeed);
    WRITE_MEMORY(0x1272ABE, float*, &m_animationData.jetWall_playbackSpeed);
    WRITE_MEMORY(0x1272B2D, float*, &m_animationData.jetWall_playbackSpeed);
    WRITE_MEMORY(0x127299C, float*, &m_animationData.jet_speedFactor);
    WRITE_MEMORY(0x1272A0B, float*, &m_animationData.jet_speedFactor);
    WRITE_MEMORY(0x1272A7A, float*, &m_animationData.jet_speedFactor);
    WRITE_MEMORY(0x1272AE9, float*, &m_animationData.jetWall_speedFactor);
    WRITE_MEMORY(0x1272B58, float*, &m_animationData.jetWall_speedFactor);

    WRITE_MEMORY(0x1272B9C, float*, &m_animationData.boost_playbackSpeed);
    WRITE_MEMORY(0x1272C0B, float*, &m_animationData.boost_playbackSpeed);
    WRITE_MEMORY(0x1272C7A, float*, &m_animationData.boost_playbackSpeed);
    WRITE_MEMORY(0x1272CE9, float*, &m_animationData.boostWall_playbackSpeed);
    WRITE_MEMORY(0x1272D58, float*, &m_animationData.boostWall_playbackSpeed);
    WRITE_MEMORY(0x1272BC7, float*, &m_animationData.boost_speedFactor);
    WRITE_MEMORY(0x1272C36, float*, &m_animationData.boost_speedFactor);
    WRITE_MEMORY(0x1272CA5, float*, &m_animationData.boost_speedFactor);
    WRITE_MEMORY(0x1272D14, float*, &m_animationData.boostWall_speedFactor);
    WRITE_MEMORY(0x1272D83, float*, &m_animationData.boostWall_speedFactor);
}
