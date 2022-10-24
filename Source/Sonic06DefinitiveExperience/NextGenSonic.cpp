#include "NextGenSonic.h"
#include "Configuration.h"
#include "Application.h"
#include "AnimationSetPatcher.h"
#include "VoiceOver.h"
#include "CustomCamera.h"

//---------------------------------------------------
// Animation
//---------------------------------------------------
void NextGenSonic::setAnimationSpeed_Sonic(NextGenAnimation& data)
{
    if (Configuration::m_physics)
    {
        data.jog_speedFactor = 2.7f;
        data.run_speedFactor = 3.2f;
        data.dash_speedFactor = 5.0f;
        data.jet_playbackSpeed = 6.0f;
        data.jet_speedFactor = -1.0f;
        data.jetWall_playbackSpeed = 6.0f;
        data.jetWall_speedFactor = -1.0f;
        data.boost_playbackSpeed = 7.0f;
        data.boost_speedFactor = -1.0f;
        data.boostWall_playbackSpeed = 7.0f;
        data.boostWall_speedFactor = -1.0f;
    }
    else
    {
        data.run_speedFactor = 6.0f;
        data.dash_speedFactor = 9.0f;
    }
}

void NextGenSonic::setAnimationSpeed_Elise(NextGenAnimation& data)
{
    if (Configuration::m_physics)
    {
        data.run_speedFactor = 5.0f;
        data.dash_speedFactor = 7.0f;
    }
    else
    {
        data.run_speedFactor = 6.0f;
        data.dash_speedFactor = 9.0f;
    }
}

//---------------------------------------------------
// Main Variables
//---------------------------------------------------
bool NextGenSonic::m_isElise = false;

bool NextGenSonic::m_bounced = false;
bool NextGenSonic::m_isStomping = false;
float const cSonic_bouncePower = 18.0f;
float const cSonic_bouncePowerBig = 23.0f;

bool NextGenSonic::m_isSquatKick = false;
bool NextGenSonic::m_isBrakeFlip = false;
Eigen::Vector3f NextGenSonic::m_brakeFlipDir(0, 0, 1);
float NextGenSonic::m_squatKickSpeed = 0.0f;
float const cSonic_squatKickPressMaxTime = 0.3f;

bool slidingEndWasSliding = false;
bool NextGenSonic::m_isSpindash = false;
bool NextGenSonic::m_isSliding = false;
float NextGenSonic::m_slidingTime = 0.0f;
float NextGenSonic::m_slidingSpeed = 0.0f;
float const cSonic_slidingTime = 3.0f;
float const cSonic_slidingSpeedMin = 10.0f;
float const cSonic_slidingSpeedMax = 16.0f;
float const cSonic_spindashTime = 3.0f;
float const cSonic_spindashSpeed = 30.0f;

float NextGenSonic::m_xHeldTimer = 0.0f;
bool NextGenSonic::m_enableAutoRunAction = true;

char const* ef_ch_sng_bound = "ef_ch_sng_bound";
char const* ef_ch_sps_bound = "ef_ch_sps_bound";
char const* ef_ch_sng_bound_down = "ef_ch_sng_bound_down";
char const* ef_ch_sps_bound_down = "ef_ch_sps_bound_down";
char const* ef_ch_sng_bound_strong = "ef_ch_sng_bound_strong";
char const* ef_ch_sng_spindash = "ef_ch_sng_spindash";

//---------------------------------------------------
// Unlimited Gauge
//---------------------------------------------------
HOOK(void, __fastcall, NextGenSonic_CHudSonicStageUpdate, 0x1098A50, Sonic::CGameObject* This, void* Edx, const hh::fnd::SUpdateInfo& in_rUpdateInfo)
{
    // Always clamp boost to 100
    *Common::GetPlayerBoost() = 100.0f;
    originalNextGenSonic_CHudSonicStageUpdate(This, Edx, in_rUpdateInfo);
}

//---------------------------------------------------
// CSonicStateHomingAttack
//---------------------------------------------------
HOOK(int, __fastcall, NextGenSonic_CSonicStateHomingAttackBegin, 0x1232040, void* This)
{
    // Use unique animation for homing attack for Elise
    if (NextGenSonic::m_isElise && !Common::IsPlayerSuper())
    {
        WRITE_MEMORY(0x1232056, char*, AnimationSetPatcher::HomingAttackLoop);
    }
    else
    {
        WRITE_MEMORY(0x1232056, uint32_t, 0x15F84E8); // JumpBall
    }

    // For Sonic's bounce bracelet
    NextGenSonic::m_bounced = false;

    return originalNextGenSonic_CSonicStateHomingAttackBegin(This);
}

HOOK(void, __fastcall, NextGenSonic_CSonicStateHomingAttackAfterAdvance, 0x1118600, void* This)
{
    bool bDown, bPressed, bReleased;
    NextGenPhysics::getActionButtonStates(bDown, bPressed, bReleased);

    if (bPressed && !Common::GetSonicStateFlags()->OutOfControl)
    {
        if (!NextGenPhysics::checkUseLightSpeedDash())
        {
            StateManager::ChangeState(StateAction::Stomping, *PLAYER_CONTEXT);
            NextGenSonic::m_xHeldTimer = 0.0f;
        }
        return;
    }

    originalNextGenSonic_CSonicStateHomingAttackAfterAdvance(This);
}

//---------------------------------------------------
// CSonicStateJumpBall
//---------------------------------------------------
HOOK(char*, __fastcall, NextGenSonic_CSonicStateJumpBallBegin, 0x11BCBE0, void* This)
{
    if (NextGenSonic::m_bounced)
    {
        // Play bound_down pfx
        WRITE_MEMORY(0x11BCDA7, char**, &ef_ch_sng_bound_down);
        WRITE_MEMORY(0x11BCCFC, char**, &ef_ch_sps_bound_down); // Super

        if (Configuration::m_physics)
        {
            // Re-enable jumpball collision
            WRITE_MEMORY(0x11BCC4B, uint8_t, 0xC1);
        }
    }
    else
    {
        WRITE_MEMORY(0x11BCDA7, uint32_t, 0x1E61D6C);
        WRITE_MEMORY(0x11BCCFC, uint32_t, 0x1E61D00); // Super

        if (Configuration::m_physics)
        {
            // Disable jumpball collision
            WRITE_MEMORY(0x11BCC4B, uint8_t, 0x71);
        }
    }

    return originalNextGenSonic_CSonicStateJumpBallBegin(This);
}

//---------------------------------------------------
// CSonicStateSliding
//---------------------------------------------------
HOOK(int, __fastcall, NextGenSonic_CSonicStateSlidingBegin, 0x11D7110, void* This)
{
    if (NextGenSonic::m_isSpindash)
    {
        // Spin animation over slide
        WRITE_MEMORY(0x11D7124, uint32_t, 0x15F84F4);
        WRITE_MEMORY(0x11D6E6A, uint32_t, 0x15F84F4);
        WRITE_MEMORY(0x11D6EDB, uint32_t, 0x15F84F4);

        // Disable sliding sfx and voice
        WRITE_MEMORY(0x11D722C, int, -1);
        WRITE_MEMORY(0x11D72DC, int, -1);

        // Use spindash pfx
        WRITE_MEMORY(0x11D6A59, uint8_t, 0x30);
        WRITE_MEMORY(0x11D6A0A, char**, &ef_ch_sng_spindash);
        WRITE_MEMORY(0x11D6A80, char**, &ef_ch_sng_spindash);

        // Play trail effect
        Common::SonicContextRequestLocusEffect();
    }
    else
    {
        // Original sliding animation
        WRITE_MEMORY(0x11D7124, uint32_t, 0x15E69CC);
        WRITE_MEMORY(0x11D6E6A, uint32_t, 0x15E69C4);
        WRITE_MEMORY(0x11D6EDB, uint32_t, 0x15E69CC);

        // Sliding sfx and voice
        WRITE_MEMORY(0x11D722C, uint32_t, 2002032);
        WRITE_MEMORY(0x11D72DC, uint32_t, 3002016);

        // Original sliding pfx
        WRITE_MEMORY(0x11D6A59, uint8_t, 0x18);
        WRITE_MEMORY(0x11D6A0A, uint32_t, 0x1E61E00);
        WRITE_MEMORY(0x11D6A80, uint32_t, 0x1E61DA8);

        NextGenSonic::m_isSliding = true;

        // Get current sliding speed
        Eigen::Vector3f playerVelocity(0, 0, 0);
        if (Common::GetPlayerVelocity(playerVelocity))
        {
            NextGenSonic::m_slidingSpeed = playerVelocity.norm();
            NextGenSonic::m_slidingSpeed = max(NextGenSonic::m_slidingSpeed, cSonic_slidingSpeedMin);
            NextGenSonic::m_slidingSpeed = min(NextGenSonic::m_slidingSpeed, cSonic_slidingSpeedMax);
        }
    }

    NextGenSonic::m_slidingTime = NextGenSonic::m_isSpindash ? cSonic_spindashTime : cSonic_slidingTime;
    return originalNextGenSonic_CSonicStateSlidingBegin(This);
}

HOOK(void, __fastcall, NextGenSonic_CSonicStateSlidingAdvance, 0x11D69A0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    originalNextGenSonic_CSonicStateSlidingAdvance(This);

    bool bDown, bPressed, bReleased;
    NextGenPhysics::getActionButtonStates(bDown, bPressed, bReleased);
    if (bPressed || NextGenSonic::m_slidingTime - This->m_Time <= 0.0f)
    {
        if (bPressed && NextGenPhysics::checkUseLightSpeedDash())
        {
            // Pressed X button and use light speed dash
            return;
        }

        if (NextGenSonic::m_isSpindash)
        {
            // Cancel spindash, this will still do sweep kick and will also allow jumping before it
            StateManager::ChangeState(StateAction::Walk, *PLAYER_CONTEXT);
            return;
        }
        else
        {
            // Cancel sliding
            slidingEndWasSliding = NextGenSonic::m_isSliding;
            StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
            return;
        }
    }

    // For 2D slide/spindash, there's one frame delay before Sonic can goto max speed, lower the minSpeed
    float minSpeed = (NextGenSonic::m_isSpindash ? cSonic_spindashSpeed : cSonic_slidingSpeedMin) - 5.0f;
    minSpeed = Common::IsPlayerIn2D() ? 2.0f : minSpeed;

    Eigen::Vector3f playerVelocity;
    bool result = Common::IsPlayerIn2D() ? Common::GetPlayerTargetVelocity(playerVelocity) : Common::GetPlayerVelocity(playerVelocity);
    if (!result || playerVelocity.norm() <= minSpeed)
    {
        slidingEndWasSliding = NextGenSonic::m_isSliding;
        StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
        return;
    }
}

void __declspec(naked) NextGenSonic_CSonicStateSlidingEnd()
{
    static uint32_t returnAddress = 0x11D702D;
    __asm
    {
        // Original function
        mov     eax, [edi + 534h]

        // Set spindash/sliding state
        mov     NextGenSonic::m_isSliding, 0
        mov     NextGenSonic::m_isSpindash, 0
        jmp     [returnAddress]
    }
}

void __declspec(naked) NextGenSonic_slidingHorizontalTargetVel2D()
{
    static uint32_t returnAddress = 0x11D98AC;
    __asm
    {
        // Get overrided target velocity
        mov     ecx, esi
        call    NextGenSonic::applySlidingHorizontalTargetVel
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        jmp     [returnAddress]
    }
}

void __declspec(naked) NextGenSonic_slidingHorizontalTargetVel3D()
{
    static uint32_t returnAddress = 0x11D953E;
    __asm
    {
        // Get overrided target velocity
        mov     ecx, ebx
        call    NextGenSonic::applySlidingHorizontalTargetVel
        test    al, al
        jnz     jump

        // Original target velocity fallback code
        movaps  xmm0, [esp + 10h]
        movaps  xmmword ptr[ebx + 2A0h], xmm0

        jump:
        jmp     [returnAddress]
    }
}

//---------------------------------------------------
// CSonicStateSlidingEnd
//---------------------------------------------------
HOOK(int, __fastcall, NextGenSonic_CSonicStateSlidingEndBegin, 0x1230F80, void* This)
{
    // For Sonic only, do a flip if no stick input
    Eigen::Vector3f inputDirection;
    if (Common::GetWorldInputDirection(inputDirection) && inputDirection.isZero())
    {
        // Do brake flip animation
        NextGenSonic::m_isBrakeFlip = true;
        WRITE_MEMORY(0x1230F88, char*, AnimationSetPatcher::BrakeFlip);
    }
    else
    {
        // Original SlidingToWalk animation
        WRITE_MEMORY(0x1230F88, uint32_t, 0x15E6A00);
    }

    return originalNextGenSonic_CSonicStateSlidingEndBegin(This);
}

HOOK(int*, __fastcall, NextGenSonic_CSonicStateSlidingEndAdvance, 0x1230EE0, void* This)
{
    int* result = originalNextGenSonic_CSonicStateSlidingEndAdvance(This);
    if (NextGenSonic::m_isBrakeFlip)
    {
        if (!slidingEndWasSliding)
        {
            // Only detect B-action during the flip, not normal SlidingEnd
            NextGenSonic::bActionHandlerImpl();
        }

        // Enforce brake flip rotation
        alignas(16) float dir[4] = { NextGenSonic::m_brakeFlipDir.x(), NextGenSonic::m_brakeFlipDir.y(), NextGenSonic::m_brakeFlipDir.z(), 0 };
        NextGenPhysics::applyCSonicRotationAdvance(This, dir);
    }
    return result;
}

HOOK(int*, __fastcall, NextGenSonic_CSonicStateSlidingEndEnd, 0x1230E60, void* This)
{
    NextGenSonic::m_isBrakeFlip = false;
    return originalNextGenSonic_CSonicStateSlidingEndEnd(This);
}

//---------------------------------------------------
// CSonicStateStomping
//---------------------------------------------------
HOOK(int, __fastcall, NextGenSonic_CSonicStateStompingBegin, 0x1254CA0, void* This)
{
    NextGenSonic::m_isStomping = true;
    return originalNextGenSonic_CSonicStateStompingBegin(This);
}

//---------------------------------------------------
// CSonicStateSquat
//---------------------------------------------------
SharedPtrTypeless spinDashSoundHandle_Sonic;
SharedPtrTypeless spinDashPfxHandle_Sonic;
HOOK(int*, __fastcall, NextGenSonic_CSonicStateSquatBegin, 0x1230A30, void* This)
{
    // Play spindash pfx
    void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
    Common::fCGlitterCreate(*PLAYER_CONTEXT, spinDashPfxHandle_Sonic, matrixNode, "ef_ch_sng_spincharge", 1);

    // Play spindash charge sfx
    Common::SonicContextPlaySound(spinDashSoundHandle_Sonic, 2002042, 1);
    return originalNextGenSonic_CSonicStateSquatBegin(This);
}

HOOK(void, __fastcall, NextGenSonic_CSonicStateSquatAdvance, 0x1230B60, void* This)
{
    originalNextGenSonic_CSonicStateSquatAdvance(This);
    Eigen::Vector3f worldDirection;
    if (!Common::GetPlayerWorldDirection(worldDirection, true)) return;

    // Allow changing Sonic's rotation when charging spindash
    alignas(16) float dir[4] = { worldDirection.x(), worldDirection.y(), worldDirection.z(), 0 };
    NextGenPhysics::applyCSonicRotationAdvance(This, dir);
}

HOOK(int*, __fastcall, NextGenSonic_CSonicStateSquatEnd, 0x12309A0, void* This)
{
    // Stop spindash pfx
    Common::fCGlitterEnd(*PLAYER_CONTEXT, spinDashPfxHandle_Sonic, false);

    spinDashSoundHandle_Sonic.reset();
    if (NextGenSonic::m_isSpindash)
    {
        // Play spindash launch sfx
        static SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 80041024, 1);
    }
    return originalNextGenSonic_CSonicStateSquatEnd(This);
}

//---------------------------------------------------
// CSonicStateSquatKick
//---------------------------------------------------
SharedPtrTypeless squatKickPfxHandle_Sonic;
HOOK(int*, __fastcall, NextGenSonic_CSonicStateSquatKickBegin, 0x12526D0, void* This)
{
    // Don't allow direction change for squat kick
    WRITE_MEMORY(0x11D943D, uint8_t, 0xEB);

    // Don't play voice for Japanese
    bool isJapaneseVoice = Common::GetVoiceLanguageType() == LT_Japanese;
    if (!isJapaneseVoice)
    {
        WRITE_MEMORY(0x1252740, uint32_t, 3002020);
    }
    else
    {
        WRITE_MEMORY(0x1252740, int, -1);
    }

    // Get initial brake flip direction
    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (Common::GetPlayerTransform(playerPosition, playerRotation))
    {
        NextGenSonic::m_brakeFlipDir = playerRotation * Eigen::Vector3f::UnitZ();
    }

    // Get current speed so we can keep it
    Eigen::Vector3f playerVelocity;
    if (Common::GetPlayerVelocity(playerVelocity))
    {
        playerVelocity.y() = 0.0f;
        NextGenSonic::m_squatKickSpeed = playerVelocity.norm();
    }

    // Play squat kick sfx and pfx
    static SharedPtrTypeless soundHandle;
    Common::SonicContextPlaySound(soundHandle, 80041021, 1);
    void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x10);
    Common::fCGlitterCreate(*PLAYER_CONTEXT, squatKickPfxHandle_Sonic, matrixNode, "ef_ch_sng_yh1_squatkick", 1);

    NextGenSonic::m_isSquatKick = true;
    return originalNextGenSonic_CSonicStateSquatKickBegin(This);
}

HOOK(void, __fastcall, NextGenSonic_CSonicStateSquatKickAdvance, 0x1252810, void* This)
{
    originalNextGenSonic_CSonicStateSquatKickAdvance(This);

    // Lock squat kick's rotation if not moving
    if (NextGenSonic::m_squatKickSpeed == 0.0f)
    {
        alignas(16) float dir[4] = { NextGenSonic::m_brakeFlipDir.x(), NextGenSonic::m_brakeFlipDir.y(), NextGenSonic::m_brakeFlipDir.z(), 0 };
        NextGenPhysics::applyCSonicRotationAdvance(This, dir);
    }
}

bool __fastcall NextGenSonic_CSonicStateSquatKickAdvanceTransitionOutImpl(char const* name)
{
    if (strcmp(name, "Stand") == 0 || strcmp(name, "Walk") == 0)
    {
        CSonicStateFlags* flags = Common::GetSonicStateFlags();
        if (!flags->KeepRunning)
        {
            Eigen::Vector3f inputDirection;
            if (Common::GetWorldInputDirection(inputDirection) && inputDirection.isZero())
            {
                // Stops Sonic completely if no stick input
                slidingEndWasSliding = NextGenSonic::m_isSliding;
                StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
                return true;
            }
        }
    }

    return false;
}

void __declspec(naked) NextGenSonic_CSonicStateSquatKickAdvanceTransitionOut()
{
    static uint32_t returnAddress = 0x1252924;
    static uint32_t sub_E4FF30 = 0xE4FF30;
    __asm
    {
        push    eax
        push    ecx

        mov     ecx, [eax]
        call    NextGenSonic_CSonicStateSquatKickAdvanceTransitionOutImpl
        mov     bl, al

        pop     ecx
        pop     eax

        test    bl, bl
        jnz     jump
        call    [sub_E4FF30]

        jump:
        jmp     [returnAddress]
    }
}

HOOK(int*, __fastcall, NextGenSonic_CSonicStateSquatKickEnd, 0x12527B0, void* This)
{
    // Unlock direction change for sliding/spindash
    WRITE_MEMORY(0x11D943D, uint8_t, 0x74);

    // Stop squat kick pfx
    Common::fCGlitterEnd(*PLAYER_CONTEXT, squatKickPfxHandle_Sonic, false);

    NextGenSonic::m_isSquatKick = false;
    return originalNextGenSonic_CSonicStateSquatKickEnd(This);
}

//---------------------------------------------------
// X Button Action
//---------------------------------------------------
HOOK(bool, __stdcall, NextGenSonic_BActionHandler, 0xDFF660, CSonicContext* context, bool buttonHoldCheck)
{
    // Reset bounce bracelet status
    if (!NextGenSonic::m_isStomping)
    {
        NextGenSonic::m_bounced = false;
    }
    NextGenSonic::m_isStomping = false;

    // Handle X button action
    bool result = originalNextGenSonic_BActionHandler(context, buttonHoldCheck);
    if (result || Common::IsPlayerControlLocked())
    {
        NextGenSonic::m_xHeldTimer = 0.0f;
        return result;
    }

    return NextGenSonic::bActionHandlerImpl();
}

HOOK(int, __fastcall, NextGenSonic_MsgRestartStage, 0xE76810, uint32_t* This, void* Edx, void* message)
{
    int result = originalNextGenSonic_MsgRestartStage(This, Edx, message);

    // For Elise's shield
    if (NextGenSonic::m_isElise)
    {
        *Common::GetPlayerBoost() = Common::GetPlayerMaxBoost();
    }

    // Re-enable auto run actions (squat kick/bounce bracelet)
    NextGenSonic::m_enableAutoRunAction = true;

    return result;
}

HOOK(void, __fastcall, NextGenSonic_MsgNotifyObjectEvent, 0xEA4F50, void* This, void* Edx, uint32_t a2)
{
    uint32_t* pEvent = (uint32_t*)(a2 + 16);
    uint32_t* pObject = (uint32_t*)This;

    // Only use [1001-2000] events
    if (*pEvent > 1000 && *pEvent <= 2000)
    {
        switch (*pEvent)
        {
        // Disable action in auto run (mach speed)
        case 1001:
        {
            NextGenSonic::m_enableAutoRunAction = false;
            break;
        }
        }
    }

    originalNextGenSonic_MsgNotifyObjectEvent(This, Edx, a2);
}

bool NextGenSonic::bActionHandlerImpl()
{
    Eigen::Vector3f playerVelocity;
    if (!Common::GetPlayerVelocity(playerVelocity))
    {
        return false;
    }

    CSonicStateFlags* flags = Common::GetSonicStateFlags();
    if (flags->KeepRunning && !m_enableAutoRunAction)
    {
        // Cannot use any action during auto run
        return false;
    }

    bool moving = playerVelocity.norm() > 0.2f;
    bool canUseSpindash = !moving || (Configuration::m_rapidSpindash && !flags->KeepRunning);

    bool bDown, bPressed, bReleased; 
    NextGenPhysics::getActionButtonStates(bDown, bPressed, bReleased);
    if (bDown)
    {
        // Standing still and held B for a while (Spin Dash)
        if (canUseSpindash && m_xHeldTimer > cSonic_squatKickPressMaxTime)
        {
            if (!m_isElise)
            {
                StateManager::ChangeState(StateAction::Squat, *PLAYER_CONTEXT);
                m_xHeldTimer = 0.0f;
                return true;
            }
        }

        // Remember how long we held X
        // NOTE: Apparently this code always runs at 60fps
        // If 30fps this will run twice per frame!
        m_xHeldTimer += 1.0f / 60.0f;
    }
    else
    {
        if (bReleased && !flags->OnWater)
        {
            if (m_xHeldTimer <= cSonic_squatKickPressMaxTime)
            {
                if (!m_isElise)
                {
                    // Release X without holding it for too long (Squat Kick)
                    StateManager::ChangeState(StateAction::SquatKick, *PLAYER_CONTEXT);
                    m_xHeldTimer = 0.0f;
                    return true;
                }
            }
            else if (moving && !flags->KeepRunning && m_xHeldTimer > cSonic_squatKickPressMaxTime)
            {
                if (!m_isElise && Configuration::m_rapidSpindash)
                {
                    // Disable anti-gravity if rapid spindash is enabled
                }
                else
                {
                    // Moving and released X (Anti-Gravity)
                    StateManager::ChangeState(StateAction::Sliding, *PLAYER_CONTEXT);
                    m_xHeldTimer = 0.0f;
                    return true;
                }
            }
        }

        m_xHeldTimer = 0.0f;
    }

    return false;
}

//---------------------------------------------------
// Bounce Bracelet
//---------------------------------------------------
void __declspec(naked) bounceBraceletASMImpl()
{
    static uint32_t returnAddress = 0x1254B77;
    __asm
    {
        call    NextGenSonic::bounceBraceletImpl
        jmp     [returnAddress]
    }
}

void NextGenSonic::bounceBraceletImpl()
{
    if (!*pModernSonicContext) return;

    // Can't do bounce when auto run
    CSonicStateFlags* flags = Common::GetSonicStateFlags();
    if (flags->KeepRunning) return;

    // Enable homing attack flag
    flags->EnableHomingAttack = true;

    // Bounce up
    float* velocity = (float*)((uint32_t)*pModernSonicContext + 0x290);
    velocity[0] = 0.0f;
    velocity[1] = NextGenSonic::m_bounced ? cSonic_bouncePowerBig : cSonic_bouncePower;
    if (flags->OnWater)
    {
        velocity[1] -= 5.0f;
    }
    velocity[2] = 0.0f;
    NextGenSonic::m_bounced = true;
    StateManager::ChangeState(StateAction::Jump, *pModernSonicContext);

    // Set out of control
    FUNCTION_PTR(int, __stdcall, SetOutOfControl, 0xE5AC00, CSonicContext * context, float duration);
    SetOutOfControl(*pModernSonicContext, 0.1f);
}

//---------------------------------------------------
// Spindashing
//---------------------------------------------------
void __declspec(naked) startSpindash()
{
    static uint32_t returnAddress = 0x1230C39;
    __asm
    {
        // Original function
        mov     byte ptr[ebx + 5E8h], 1
        mov[ebx + 5E9h], al

        // Set spindash state
        mov     NextGenSonic::m_isSpindash, 1

        // Give Sonic the initial nudge
        mov     ecx, ebx
        call    NextGenSonic::applySpindashImpulse

        jmp     [returnAddress]
    }
}

bool __fastcall NextGenSonic::applySpindashImpulse(void* context)
{
    // This function is necessary for 2D otherwise Sonic will stop immediately
    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (!Common::GetPlayerTransform(playerPosition, playerRotation)) return false;

    alignas(16) MsgApplyImpulse message {};
    message.m_position = playerPosition;
    message.m_impulse = playerRotation * Eigen::Vector3f::UnitZ();
    message.m_impulseType = ImpulseType::None;
    message.m_outOfControl = 0.0f;
    message.m_notRelative = true;
    message.m_snapPosition = false;
    message.m_pathInterpolate = false;
    message.m_alwaysMinusOne = -1.0f;

    if (m_isSpindash)
    {
        message.m_impulse *= cSonic_spindashSpeed;
    }
    else
    {
        message.m_impulse *= m_slidingSpeed;
    }

    Common::ApplyPlayerApplyImpulse(message);

    return true;
}

bool __fastcall NextGenSonic::applySlidingHorizontalTargetVel(void* context)
{
    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (!Common::GetPlayerTransform(playerPosition, playerRotation)) return false;
    Eigen::Vector3f playerDir = playerRotation * Eigen::Vector3f::UnitZ();

    // If not moving, don't update brake flip direction
    if (m_isBrakeFlip || (m_isSquatKick && m_squatKickSpeed == 0.0f))
    {
        // Stop Sonic immediately
        float* horizontalVel = (float*)((uint32_t)context + 0x290);
        float* horizontalTargetVel = (float*)((uint32_t)context + 0x2A0);
        horizontalVel[0] = 0;
        horizontalVel[2] = 0;
        horizontalTargetVel[0] = 0;
        horizontalTargetVel[2] = 0;

        return true;
    }

    m_brakeFlipDir = playerDir;
    bool superForm = Common::IsPlayerSuper();
    if (m_isSquatKick)
    {
        // Keep velocity with squat kick
        playerDir *= m_squatKickSpeed;
    }
    else if (m_isSpindash)
    {
        playerDir *= cSonic_spindashSpeed;

        // Double speed for super and normal physics
        if (superForm || !Configuration::m_physics)
        {
            playerDir *= Common::IsPlayerIn2D() ? 1.5f : 2.0f;
        }
    }
    else
    {
        // Use default sliding behavior for super and normal physics
        if (superForm || !Configuration::m_physics)
        {
            return false;
        }

        playerDir *= m_slidingSpeed;
    }

    // For 2D we have to override the actual velocity (+0x290)
    // For 3D we have to override target velocity (+0x2A0)
    float* horizontalVel = (float*)((uint32_t)context + (Common::IsPlayerIn2D() ? 0x290 : 0x2A0));
    horizontalVel[0] = playerDir.x();
    if (Common::IsPlayerIn2D() && !m_isSquatKick)
    {
        horizontalVel[1] = playerDir.y();
    }
    horizontalVel[2] = playerDir.z();

    return true;
}

//-------------------------------------------------------
// Rechargable Shield
//-------------------------------------------------------
void __declspec(naked) NextGenSonic_groundBoostSuperSonicOnly()
{
    static uint32_t returnAddress = 0xDFF270;
    static uint32_t failAddress = 0xDFF2CB;
    __asm
    {
        // disable air boost for normal Sonic
        mov     eax, [ebx + 534h]
        mov     eax, [eax + 4]
        cmp     byte ptr[eax + 6Fh], 0
        jz      jump

        // original function
        movss   xmm0, dword ptr[ebx + 5BCh]
        jmp     [returnAddress]
        
        jump:
        jmp     [failAddress]
    }
}

void __declspec(naked) NextGenSonic_airBoostSuperSonicOnly()
{
    static uint32_t returnAddress = 0xDFE094;
    static uint32_t failAddress = 0xDFDFE6;
    __asm
    {
        // disable air boost for normal Sonic
        mov     eax, [esi + 534h]
        mov     eax, [eax + 4]
        cmp     byte ptr[eax + 6Fh], 0
        jz      jump

        // original function
        movss   xmm0, dword ptr[esi + 5BCh]
        jmp     [returnAddress]
        
        jump:
        jmp     [failAddress]
    }
}

bool NextGenSonic::m_isShield = false;
SharedPtrTypeless shieldSoundHandle_Elise;
SharedPtrTypeless shieldPfxHandle_Elise;
HOOK(int*, __fastcall, NextGenSonic_CSonicStatePluginBoostBegin, 0x1117A20, void* This)
{
    if (Common::IsPlayerSuper())
    {
        return originalNextGenSonic_CSonicStatePluginBoostBegin(This);
    }

    // Play shield sfx
    Common::SonicContextPlaySound(shieldSoundHandle_Elise, 2002030, 1);
    static uint32_t sub_DFAC30 = 0xDFAC30;
    __asm
    {
        mov     eax, This
        mov     eax, [eax + 8]
        call    [sub_DFAC30]
    }

    // Play shield pfx
    void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
    Common::fCGlitterCreate(*PLAYER_CONTEXT, shieldPfxHandle_Elise, matrixNode, "ef_ch_sng_yh1_boost2", 5);
    
    // Enable boost collision
    Common::SonicContextSetCollision(SonicCollision::TypeSonicBoost, true);

    // Float on water
    Common::GetSonicStateFlags()->AcceptBuoyancyForce = true;
    WRITE_NOP(0x119C0E5, 2); // float even if speed is 0
    WRITE_NOP(0xDED132, 3);  // don't reset AcceptBuoyancyForce
    WRITE_MEMORY(0xDFB98A, uint8_t, 0x90, 0x90, 0xF3, 0x0F, 0x10, 0x05, 0x00,   // always use BuoyantForceMaxGravityRate
        0xB5, 0x58, 0x01, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90);      // and set it as 10.0f
    WRITE_MEMORY(0x119C00E, uint8_t, 0x0F, 0x57, 0xC0, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90); // Set WaterDecreaseForce = 0

    // Push CObjCscLavaRide down with boost collision
    WRITE_MEMORY(0xF1E7D6, SonicCollision, TypeSonicBoost);

    return nullptr;
}

bool __fastcall NextGenSonic_CanActivateEliseShield()
{
    float* currentBoost = Common::GetPlayerBoost();
    Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();

    return !Common::IsPlayerControlLocked() &&
           !Common::IsPlayerSuper() &&
           !Common::IsPlayerDead() &&
           *currentBoost > 0.0f &&
           padState->IsDown(Sonic::EKeyState::eKeyState_RightTrigger);
}

void __declspec(naked) NextGenSonic_CSonicStatePluginBoostAdvance()
{
    static uint32_t returnAddress = 0x1117731;
    static uint32_t successAddress = 0x11178F1;
    __asm
    {
        push    eax
        push    ecx
        call    NextGenSonic_CanActivateEliseShield
        xor     al, 1 // flip it so we can exit for false
        mov     byte ptr[ebp + 1Ch], al
        pop     ecx
        pop     eax

        // Use normal advance for Super Sonic
        cmp     byte ptr[ecx + 6Fh], 0
        jnz     jump
        jmp     [successAddress]

        // original function
        jump:
        mov     byte ptr[ebp + 1Ch], 0
        mov     [esp + 0Fh], 0
        jmp     [returnAddress]
    }
}

HOOK(void, __fastcall, NextGenSonic_CSonicStatePluginBoostEnd, 0x1117900, void* This)
{
    if (Common::IsPlayerSuper())
    {
        originalNextGenSonic_CSonicStatePluginBoostEnd(This);
    }

    NextGenSonic::m_isShield = false;

    // Stop sfx
    Common::SonicContextPlaySound(shieldSoundHandle_Elise, 3002089, 1);
    __asm
    {
        mov     edi, This
        mov     edi, [edi + 8]
        mov     eax, [edi]
        mov     edx, [eax + 9Ch]
        lea     ecx, [edi + 14B4h]
        push    ecx
        mov     ecx, edi
        call    edx
    }

    // Stop pfx
    Common::fCGlitterEnd(*PLAYER_CONTEXT, shieldPfxHandle_Elise, false);

    // Disable boost collision
    Common::SonicContextSetCollision(SonicCollision::TypeSonicBoost, false);

    // Disable float on water
    Eigen::Vector3f playerVelocity;
    if (Common::GetPlayerVelocity(playerVelocity))
    {
        playerVelocity.y() = 0.0f;
        if (playerVelocity.norm() == 0.0f)
        {
            Common::GetSonicStateFlags()->AcceptBuoyancyForce = false;
        }
    }
    WRITE_MEMORY(0x119C0E5, uint8_t, 0x76, 0x59);
    WRITE_MEMORY(0xDED132, uint8_t, 0x88, 0x59, 0x59);
    WRITE_MEMORY(0xDFB98A, uint8_t, 0x74, 0x3A, 0x8B, 0x86, 0x7C, 0x02, 0x00,
        0x00, 0x68, 0xA6, 0x00, 0x00, 0x00, 0xE8, 0x54, 0xF0, 0x73, 0xFF);
    WRITE_MEMORY(0x119C00E, uint8_t, 0x68, 0xA7, 0x00, 0x00, 0x00, 0xE8, 0xD8, 0xE9, 0x39, 0xFF);

    // Revert CObjCscLavaRide to stomp collision
    WRITE_MEMORY(0xF1E7D6, SonicCollision, TypeSonicStomping);
}

float NextGenSonic::m_shieldDecRate = 10.0f;
float NextGenSonic::m_shieldRechargeRate = 50.0f;
float NextGenSonic::m_shieldNoChargeTime = 0.0f;
float NextGenSonic::m_shieldNoChargeDelay = 0.5f;
HOOK(void, __fastcall, NextGenSonic_CSonicUpdateEliseShield, 0xE6BF20, void* This, void* Edx, float* dt)
{
    // Ignore for Super Sonic
    if (Common::IsPlayerSuper())
    {
        // Enable stomping
        WRITE_MEMORY(0xDFDDB3, uint8_t, 0x75);

        originalNextGenSonic_CSonicUpdateEliseShield(This, Edx, dt);
        return;
    }
    else
    {
        // Disable stomping
        WRITE_MEMORY(0xDFDDB3, uint8_t, 0xEB);
    }

    // Handle shield start and end
    if (NextGenSonic_CanActivateEliseShield())
    {
        if (!NextGenSonic::m_isShield)
        {
            NextGenSonic::m_isShield = true;
            Common::SonicContextAddPlugin("Boost");
        }
    }
    else
    {
        NextGenSonic::m_isShield = false;
    }

    // Handle boost gauge
    float* currentBoost = Common::GetPlayerBoost();
    Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
    if (NextGenSonic::m_isShield || !Common::IsPlayerGrounded() || padState->IsDown(Sonic::EKeyState::eKeyState_RightTrigger))
    {
        NextGenSonic::m_shieldNoChargeTime = NextGenSonic::m_shieldNoChargeDelay;
        if (NextGenSonic::m_isShield)
        {
            *currentBoost = max(0.0f, *currentBoost - NextGenSonic::m_shieldDecRate * *dt);
        }
    }
    else
    {
        // Delay before actually recharge
        NextGenSonic::m_shieldNoChargeTime = max(0.0f, NextGenSonic::m_shieldNoChargeTime - *dt);
        if (NextGenSonic::m_shieldNoChargeTime == 0.0f)
        {
            float const maxBoost = Common::GetPlayerMaxBoost();
            *currentBoost = min(maxBoost, *currentBoost + NextGenSonic::m_shieldRechargeRate * *dt);
        }
    }

    originalNextGenSonic_CSonicUpdateEliseShield(This, Edx, dt);
}

//-------------------------------------------------------
// Gems
//-------------------------------------------------------
S06HUD_API::SonicGemType NextGenSonic::m_sonicGemType = S06HUD_API::SonicGemType::SGT_None;
FUNCTION_PTR(bool*, __thiscall, CSingleElementChangeMaterial, 0x701CC0, Hedgehog::Mirage::CSingleElement* singleElement, hh::mr::CMaterialData* from, boost::shared_ptr<hh::mr::CMaterialData>& to);
FUNCTION_PTR(bool*, __thiscall, CSingleElementResetMaterial, 0x701830, Hedgehog::Mirage::CSingleElement* singleElement, hh::mr::CMaterialData* mat);

float const cSonic_gemDrainRate = 10.0f;
Eigen::Vector3f NextGenSonic::m_gemHoldPosition = Eigen::Vector3f::Zero();

float const cSonic_blueGemCost = 15.0f;
float const cSonic_blueGemHomingDelay = 0.15f;

float const cSonic_yellowGemCost = 100.0f;
float const cSonic_whiteGemCost = 10.0f;
float const cSonic_whiteGemSpeed = 110.0f;
float const cSonic_whiteGemDummySpeed = 60.0f;
float const cSonic_whiteGemHomingRange = 25.0f;
bool NextGenSonic::m_whiteGemEnabled = false;

bool NextGenSonic::m_purpleGemEnabled = false;
float NextGenSonic::m_purpleGemBlockTimer = 0.0f;
int NextGenSonic::m_purpleGemJumpCount = 0;
float const cSonic_purpleGemModelScale = 0.1f;
float const cSonic_purpleGemSpeedScale = 1.2f;
float const cSonic_purpleGemBlockTime = 0.5f;
float const cSonic_purpleGemJumpPower = 12.0f; // in 06 is 0.5x (9.0f) due to slow gravity, so we do 0.667x here instead

float const cSonic_greenGemCost = 30.0f;
float const cSonic_greenGemHeight = 8.0f; // 06: 4.0f
float const cSonic_greenGemRadius = 16.0f; // 06: 8.0f
float const cSonic_greenGemShockWaveInterval = 0.5f;
bool NextGenSonic::m_greenGemEnabled = false;

float const cSonic_skyGemCost = 30.0f;
float const cSonic_skyGemThrowSpeed = 40.0f;
float const cSonic_skyGemGravity = -15.0f;
float const cSonic_skyGemLaunchSpeedDummy = 20.0f;
float const cSonic_skyGemLaunchSpeed = 50.0f;
float const cSonic_skyGemDummyThrowTime = 0.3f;
bool NextGenSonic::m_skyGemEnabled = false;
bool NextGenSonic::m_skyGemLaunched = false;
bool NextGenSonic::m_skyGemCancelled = false;
boost::shared_ptr<Sonic::CGameObject> m_spSkyGemSingleton;
boost::shared_ptr<Sonic::CGameObject> m_spSkyGemLockonCursor;

char const* homingAttackAnim[] =
{
    "HomingAttackAfter1",
    "HomingAttackAfter2",
    "HomingAttackAfter3",
    "HomingAttackAfter4",
    "HomingAttackAfter5",
    "HomingAttackAfter6"
}; 

SharedPtrTypeless skyGemWaitPfxHandle;
class CObjSkyGem : public Sonic::CGameObject3D
{
    INSERT_PADDING(0x4);
    void* m_GlitterPlayer;
    boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
    Hedgehog::Math::CVector m_Position;
    Hedgehog::Math::CVector m_Velocity;
    float m_Gravity;
    float m_LifeTime;
    float m_DistTravelled;
    
public:
    CObjSkyGem
    (
        Hedgehog::Math::CVector const& _Position,
        Hedgehog::Math::CVector const& _Direction,
        float _Speed = cSonic_skyGemThrowSpeed,
        float _Gravity = cSonic_skyGemGravity
    )
        : m_Position(_Position)
        , m_Velocity(_Direction.normalized() * _Speed)
        , m_Gravity(_Gravity)
        , m_LifeTime(0.0f)
        , m_DistTravelled(0.0f)
    {}

    ~CObjSkyGem()
    {
        if (m_GlitterPlayer)
        {
            typedef void* __fastcall CGlitterPlayerDestructor(void* This, void* Edx, bool freeMem);
            CGlitterPlayerDestructor* destructorFunc = *(CGlitterPlayerDestructor**)(*(uint32_t*)m_GlitterPlayer);
            destructorFunc(m_GlitterPlayer, nullptr, true);
        }
    }

    void AddCallback
    (
        const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder, 
        Sonic::CGameDocument* pGameDocument,
        const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
    ) override
    {
        // Base on sub_1058B00
        FUNCTION_PTR(void, __thiscall, FuncNeededToMakeGlitterWork, 0xD5CB80,
            void* This,
            const Hedgehog::Base::THolder<Sonic::CWorld>&worldHolder,
            Sonic::CGameDocument * pGameDocument,
            const boost::shared_ptr<Hedgehog::Database::CDatabase>&spDatabase);
        FUNCTION_PTR(void*, __cdecl, CGlitterPlayerInit, 0x1255B40, Sonic::CGameDocument* pGameDocument);
        FuncNeededToMakeGlitterWork(this, worldHolder, pGameDocument, spDatabase);
        m_GlitterPlayer = CGlitterPlayerInit(pGameDocument);

        // Prevent fpCGameObject3DMatrixNodeChangedCallback crashing
        *(uint32_t*)((uint32_t)this + 0x1C) = 0;

        // Thrown pfx
        static SharedPtrTypeless pfxHandle;
        void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
        Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle, matrixNode, "ef_ch_sng_skythown", 1);

        // Wait pfx
        Common::fCGlitterCreate(*PLAYER_CONTEXT, skyGemWaitPfxHandle, matrixNode, "ef_ch_sng_skywait", 1);

        // Trail pfx
        Common::ObjectCGlitterPlayerOneShot(this, "ef_ch_sng_skytrail");

        // launch sfx
        static SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 80041031, 1);

        Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
        pGameDocument->AddUpdateUnit("0", this);

        m_spModel = boost::make_shared<hh::mr::CSingleElement>(hh::mr::CMirageDatabaseWrapper(spDatabase.get()).GetModelData("so_itm_gemSky"));
        AddRenderable("Object", m_spModel, false);

        UpdateTransform();
    }

    bool ProcessMessage
    (
        Hedgehog::Universe::Message& message, 
        bool flag
    ) override
    {
        //printf("[Sky Gem] Recieved message type %s\n", message.GetType());
        if (flag)
        {
            if (std::strstr(message.GetType(), "MsgRestartStage") != nullptr
             || std::strstr(message.GetType(), "MsgStageClear") != nullptr)
            {
                Kill();
                return true;
            }
        }

        return Sonic::CGameObject::ProcessMessage(message, flag);
    }

    void UpdateParallel
    (
        const Hedgehog::Universe::SUpdateInfo& updateInfo
    ) override
    {
        // Do equation of motion manually
        Hedgehog::Math::CVector const velPrev = m_Velocity;
        m_Velocity += Hedgehog::Math::CVector::UnitY() * m_Gravity * updateInfo.DeltaTime;
        Hedgehog::Math::CVector const posPrev = m_Position;
        m_Position += (velPrev + m_Velocity) * 0.5f * updateInfo.DeltaTime;
        m_DistTravelled += (m_Position - posPrev).norm();

        UpdateTransform();

        // Raycast for wall detection
        Eigen::Vector4f const rayStartPos(posPrev.x(), posPrev.y(), posPrev.z(), 1.0f);
        Eigen::Vector4f const rayEndPos(m_Position.x(), m_Position.y(), m_Position.z(), 1.0f);
        Eigen::Vector4f outPos;
        Eigen::Vector4f outNormal;
        if (Common::fRaycast(rayStartPos, rayEndPos, outPos, outNormal, *(uint32_t*)0x1E0AFB4))
        {
            Eigen::Vector3f impulse;
            float outOfControl = 0.0f;
            auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
            if (context && !context->StateFlag(eStateFlag_Dead))
            {
                float launchSpeed = cSonic_skyGemLaunchSpeed;
                for (int i = 0; i < 5; i++)
                {
                    if (Common::SolveBallisticArc
                        (
                            context->m_spMatrixNode->m_Transform.m_Position,
                            outPos.head<3>(),
                            launchSpeed,
                            context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_Gravity),
                            false,
                            impulse,
                            outOfControl
                        )
                    )
                    {
                        printf("[Sky Gem] Launch velocity {%.2f, %.2f, %.2f}, Speed = %.f, OutOfControl = %.2fs, Dist = %.2f\n", DEBUG_VECTOR3(impulse), launchSpeed, outOfControl, m_DistTravelled);
                        alignas(16) MsgApplyImpulse message {};
                        message.m_position = context->m_spMatrixNode->m_Transform.m_Position;
                        message.m_impulse = impulse;
                        message.m_impulseType = ImpulseType::JumpBoard;
                        message.m_outOfControl = outOfControl;
                        message.m_notRelative = true;
                        message.m_snapPosition = false;
                        message.m_pathInterpolate = false;
                        message.m_alwaysMinusOne = -1.0f;
                        Common::ApplyPlayerApplyImpulse(message);

                        // Sky gem move sfx
                        SharedPtrTypeless soundHandle;
                        Common::SonicContextPlaySound(soundHandle, 80041032, 1);

                        // Prevent using Sky Gem in air
                        Common::GetSonicStateFlags()->EnableAirOnceAction = false;
                        break;
                    }
                    else
                    {
                        printf("[Sky Gem] No solution for launch speed %.f\n", launchSpeed);
                        launchSpeed += 10.0f;
                    }
                }
            }

            Kill();
        }
        else
        {
            m_LifeTime += updateInfo.DeltaTime;
            if (m_LifeTime >= 5.0f)
            {
                printf("[Sky Gem] Timeout\n");
                Kill();
            }
        }
    }

    void Kill()
    {
        printf("[Sky Gem] Killed\n");
        SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
        m_spSkyGemSingleton = nullptr;
        Common::fCGlitterEnd(*PLAYER_CONTEXT, skyGemWaitPfxHandle, true);
    }

    void UpdateTransform()
    {
        // Rotate gem to correct rotation
        Hedgehog::Math::CVector const dir = m_Velocity.normalized();
        Hedgehog::Math::CVector dirXZ = dir; dirXZ.y() = 0.0f;
        Hedgehog::Math::CQuaternion rotYaw = Hedgehog::Math::CQuaternion::FromTwoVectors(Hedgehog::Math::CVector::UnitZ(), dirXZ.head<3>());
        Hedgehog::Math::CQuaternion rotPitch = Hedgehog::Math::CQuaternion::FromTwoVectors(dirXZ.head<3>(), dir.head<3>());
        m_spModel->m_spInstanceInfo->m_Transform = (Eigen::Translation3f(m_Position) * rotPitch * rotYaw).matrix();

        m_spMatrixNodeTransform->m_Transform.SetPosition(m_Position);
        m_spMatrixNodeTransform->NotifyChanged();
    }
};

class CObjSkyGemLockonCursor : public Sonic::CGameObject
{
    Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectLockonCursor;
    boost::shared_ptr<Sonic::CGameObjectCSD> m_spLockonCursor;
    Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneLockonCursor;

    Eigen::Vector4f m_pos;
    bool m_isOutro;

    std::mutex m_mutex;

public:
    CObjSkyGemLockonCursor(Eigen::Vector4f inPos)
        : m_pos(inPos)
        , m_isOutro(false)
    {
    }

    ~CObjSkyGemLockonCursor()
    {
        if (m_spLockonCursor)
        {
            m_spLockonCursor->SendMessage(m_spLockonCursor->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
            m_spLockonCursor = nullptr;
        }

        Chao::CSD::CProject::DestroyScene(m_projectLockonCursor.Get(), m_sceneLockonCursor);
        m_projectLockonCursor = nullptr;
    }

    void AddCallback
    (
        const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
        Sonic::CGameDocument* pGameDocument,
        const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
    ) override
    {
        Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
        pGameDocument->AddUpdateUnit("1", this);

        Sonic::CCsdDatabaseWrapper wrapper(m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());
        auto spCsdProject = wrapper.GetCsdProject("ui_lockon_cursor");
        m_projectLockonCursor = spCsdProject->m_rcProject;
        if (m_projectLockonCursor)
        {
            m_sceneLockonCursor = m_projectLockonCursor->CreateScene("cursor");
            PlayIntro();

            if (!m_spLockonCursor)
            {
                m_spLockonCursor = boost::make_shared<Sonic::CGameObjectCSD>(m_projectLockonCursor, 0.5f, "HUD_B2", false);
                Sonic::CGameDocument::GetInstance()->AddGameObject(m_spLockonCursor, "main", this);
            }
        }
    }

    bool ProcessMessage
    (
        Hedgehog::Universe::Message& message,
        bool flag
    ) override
    {
        if (flag)
        {
            if (std::strstr(message.GetType(), "MsgRestartStage") != nullptr
             || std::strstr(message.GetType(), "MsgStageClear") != nullptr)
            {
                Kill();
                return true;
            }
        }

        return Sonic::CGameObject::ProcessMessage(message, flag);
    }

    void UpdateParallel
    (
        const Hedgehog::Universe::SUpdateInfo& updateInfo
    ) override
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        // loop animation
        if (m_sceneLockonCursor && m_sceneLockonCursor->m_MotionDisableFlag)
        {
            if (m_isOutro)
            {
                Kill();
            }
            else
            {
                PlayMotion("Usual_Anim", true);
            }
        }

        // Handle position
        if (m_sceneLockonCursor)
        {
            Eigen::Vector4f screenPosition;
            Common::fGetScreenPosition(m_pos, screenPosition);
            m_sceneLockonCursor->SetPosition(screenPosition.x(), screenPosition.y());
        }
    }

    void PlayIntro()
    {
        PlayMotion("Intro_Anim", false);

        static SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 4002012, 0);
    }

    void SetPos(Eigen::Vector4f inPos)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_pos = inPos;

        if (m_isOutro)
        {
            PlayIntro();
        }
        m_isOutro = false;
    }

    void SetOutro()
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        PlayMotion("Outro_Anim", false);
        m_isOutro = true;
    }

    void PlayMotion(char const* motion, bool loop = false)
    {
        if (!m_sceneLockonCursor) return;
        m_sceneLockonCursor->SetHideFlag(false);
        m_sceneLockonCursor->SetMotion(motion);
        m_sceneLockonCursor->m_MotionDisableFlag = false;
        m_sceneLockonCursor->m_MotionFrame = 0.0f;
        m_sceneLockonCursor->m_MotionSpeed = 1.0f;
        m_sceneLockonCursor->m_MotionRepeatType = loop ? Chao::CSD::eMotionRepeatType_Loop : Chao::CSD::eMotionRepeatType_PlayOnce;
        m_sceneLockonCursor->Update();
    }

    void Kill()
    {
        printf("[LockonCursor] Killed\n");
        SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
        m_spSkyGemLockonCursor = nullptr;
    }
};

bool NextGenSonicGems_BlueGemCheck()
{
    auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
    if (context->m_ChaosEnergy >= cSonic_blueGemCost && context->m_HomingAttackTargetActorID)
    {
        context->m_ChaosEnergy -= cSonic_blueGemCost;
        return true;
    }

    return  false;
}

void __declspec(naked) NextGenSonicGems_BlueGemAirAction()
{
    static uint32_t successAddress = 0xDFFEBA;
    static uint32_t returnAddress = 0xDFFEAD;
    static uint32_t fnButtonHold = 0xD97DA0;
    __asm
    {
        // Holding Right Trigger auto homing attack
        push    esi
        mov     esi, [esi + 11Ch]
        mov     edi, 20h // Right Trigger
        call    [fnButtonHold]
        pop     esi

        test    al, al
        jz      original

        // Check if we can use blue gem
        call    NextGenSonicGems_BlueGemCheck
        test    al, al
        jnz     jump

        // original function
        original:
        mov     eax, [esi + 11Ch]
        jmp     [returnAddress]

        jump:
        jmp     [successAddress]
    }
}

bool NextGenSonicGems_WhiteGemCheck()
{
    return Sonic::Player::CPlayerSpeedContext::GetInstance()->m_ChaosEnergy >= cSonic_whiteGemCost
        && !Common::GetSonicStateFlags()->KeepRunning;
}

void __declspec(naked) NextGenSonicGems_WhiteGemAirAction()
{
    static uint32_t successAddress = 0xDFFEBA;
    static uint32_t returnAddress = 0xDFFEAD;
    static uint32_t fnButtonPress = 0xD97E00;
    __asm
    {
        // White Gem
        mov     eax, [esi + 11Ch]
        push    edi
        mov     edi, 20h // Right Trigger
        call    [fnButtonPress]
        pop     edi

        test    al, al
        jz      original

        // Check if we can use white gem
        call    NextGenSonicGems_WhiteGemCheck
        test    al, al
        jnz     jump

        // original function
        original:
        mov     eax, [esi + 11Ch]
        jmp     [returnAddress]

        jump:
        mov     NextGenSonic::m_whiteGemEnabled, 1
        jmp     [successAddress]
    }
}

bool NextGenSonicGems_PurpleGemAction()
{
    if (NextGenSonic::m_purpleGemBlockTimer <= 0.0f)
    {
        Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
        if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
        {
            Eigen::Vector3f velocity;
            Common::GetPlayerVelocity(velocity);
            if (velocity.y() < 0.0f)
            {
                NextGenSonic::m_purpleGemBlockTimer = cSonic_purpleGemBlockTime;

                // Set velocity
                velocity.y() = cSonic_purpleGemJumpPower;
                if (Common::GetSonicStateFlags()->OnWater)
                {
                    velocity.y() -= 5.0f;
                }
                Common::SetPlayerVelocity(velocity);

                // Change state
                Common::GetSonicStateFlags()->EnableAttenuateJump = true;
                StateManager::ChangeState(StateAction::Jump, *PLAYER_CONTEXT);

                // Manually play jump sfx and voice
                if (NextGenSonic::m_purpleGemJumpCount++ < 2)
                {
                    static SharedPtrTypeless soundHandle;
                    Common::SonicContextPlaySound(soundHandle, 2002027, 1);
                    VoiceOver::playJumpVoice();
                }

                return true;
            }
        }
    }

    return false;
}

void __declspec(naked) NextGenSonicGems_PuepleGemAirAction()
{
    static uint32_t returnAddress = 0xDFFEE5;
    __asm
    {
        call    NextGenSonicGems_PurpleGemAction
        jmp     [returnAddress]
    }
}

bool NextGenSonicGems_SkyGemCheck()
{
    auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
    if (context->m_ChaosEnergy >= cSonic_skyGemCost)
    {
        context->m_ChaosEnergy -= cSonic_skyGemCost;
        if (m_spSkyGemSingleton)
        {
            ((CObjSkyGem*)m_spSkyGemSingleton.get())->Kill();
        }

        // Throw CObjSkyGem
        Hedgehog::Math::CVector position = context->m_spMatrixNode->m_Transform.m_Position + Hedgehog::Math::CVector::UnitY() * 0.8f;
        Hedgehog::Math::CVector direction = context->m_HorizontalRotation * Hedgehog::Math::CVector::UnitZ();
        m_spSkyGemSingleton = boost::make_shared<CObjSkyGem>(position, direction, cSonic_skyGemLaunchSpeedDummy);
        context->m_pPlayer->m_pMember->m_pGameDocument->AddGameObject(m_spSkyGemSingleton);

        return true;
    }

    return  false;
}

void __declspec(naked) NextGenSonicGems_SkyGemAirAction()
{
    static uint32_t successAddress = 0xDFFEDC;
    static uint32_t returnAddress = 0xDFFEAD;
    static uint32_t fnButtonPress = 0xD97E00;
    __asm
    {
        // Sky Gem
        mov     eax, [esi + 11Ch]
        push    edi
        mov     edi, 20h // Right Trigger
        call    [fnButtonPress]
        pop     edi

        test    al, al
        jz      original

        // Check if we can use sky gem
        call    NextGenSonicGems_SkyGemCheck
        test    al, al
        jnz     jump

        // original function
        original:
        mov     eax, [esi + 11Ch]
        jmp     [returnAddress]

        jump:
        jmp     [successAddress]
    }
}

void NextGenSonic::ChangeGems(S06HUD_API::SonicGemType oldType, S06HUD_API::SonicGemType newType)
{
    if (!*pModernSonicContext) return;

    m_sonicGemType = newType;

    // Blue/No Gem
    DisableGem(S06HUD_API::SonicGemType::SGT_None);
    if (newType == S06HUD_API::SonicGemType::SGT_None)
    {
        WRITE_MEMORY(0xDFF268, uint8_t, 0xF3, 0x0F, 0x10, 0x83, 0xBC);
        WRITE_MEMORY(0xDFE05F, uint8_t, 0xF3, 0x0F, 0x10, 0x86, 0xBC);
    }
    else
    {
        // Disable boost for normal Sonic only
        WRITE_JUMP(0xDFF268, NextGenSonic_groundBoostSuperSonicOnly);
        WRITE_JUMP(0xDFE05F, NextGenSonic_airBoostSuperSonicOnly);
    }

    // Red Gem
    DisableGem(S06HUD_API::SonicGemType::SGT_Red);
    if (newType == S06HUD_API::SonicGemType::SGT_Red)
    {
        *(float*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x1A4 }) = 0.5f;
    }
    else
    {
        *(float*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x1A4 }) = 1.0f;
    }

    // White Gem/Blue Gem
    DisableGem(S06HUD_API::SonicGemType::SGT_White); 
    if (newType == S06HUD_API::SonicGemType::SGT_Blue)
    {
        WRITE_JUMP(0xDFFEA3, NextGenSonicGems_BlueGemAirAction);
    }
    else if (newType == S06HUD_API::SonicGemType::SGT_White)
    {
        WRITE_JUMP(0xDFFEA3, NextGenSonicGems_WhiteGemAirAction);
    }
    else if (newType == S06HUD_API::SonicGemType::SGT_Sky)
    {
        WRITE_JUMP(0xDFFEA3, NextGenSonicGems_SkyGemAirAction);
    }
    else
    {
        WRITE_MEMORY(0xDFFEA3, uint8_t, 0x84, 0xC0, 0x74, 0x3C, 0x8B);
    }

    // Purple Gem
    DisableGem(S06HUD_API::SonicGemType::SGT_Purple);

    // Handle mesh/material change
    auto const& model = Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_spCharacterModel;
   
    model->m_spModel->m_NodeGroupModels[4]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_None);
    model->m_spModel->m_NodeGroupModels[5]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_None);
    
    model->m_spModel->m_NodeGroupModels[7]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_White);
    model->m_spModel->m_NodeGroupModels[8]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_White);
    
    model->m_spModel->m_NodeGroupModels[9]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_Green);
    model->m_spModel->m_NodeGroupModels[10]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_Green);
    
    model->m_spModel->m_NodeGroupModels[11]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_Blue);
    model->m_spModel->m_NodeGroupModels[12]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_Blue);
    
    model->m_spModel->m_NodeGroupModels[13]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_Purple);
    model->m_spModel->m_NodeGroupModels[14]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_Purple);
    
    model->m_spModel->m_NodeGroupModels[15]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_Sky);
    model->m_spModel->m_NodeGroupModels[16]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_Sky);
    
    model->m_spModel->m_NodeGroupModels[17]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_Yellow);
    model->m_spModel->m_NodeGroupModels[18]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_Yellow);
    
    model->m_spModel->m_NodeGroupModels[19]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_Red);
    model->m_spModel->m_NodeGroupModels[20]->m_Visible = (m_sonicGemType == S06HUD_API::SonicGemType::SGT_Red);

    /*hh::mr::CMirageDatabaseWrapper wrapper(Sonic::CGameDocument::GetInstance()->m_pMember->m_spDatabase.get());
    boost::shared_ptr<hh::mr::CMaterialData> m1 = wrapper.GetMaterialData("ch_06_sonic_cloth");
    boost::shared_ptr<hh::mr::CMaterialData> m2 = wrapper.GetMaterialData("ch_06_sonic_belt");
    boost::shared_ptr<hh::mr::CMaterialData> m3 = wrapper.GetMaterialData("ch_06_sonic_red");
    boost::shared_ptr<hh::mr::CMaterialData> m4 = wrapper.GetMaterialData("ch_06_sonic_sole");
    boost::shared_ptr<hh::mr::CMaterialData> m5 = wrapper.GetMaterialData("invisible", 1);
    CSingleElementChangeMaterial(model.get(), m1.get(), m5);
    CSingleElementChangeMaterial(model.get(), m2.get(), m5);
    CSingleElementChangeMaterial(model.get(), m3.get(), m5);
    CSingleElementChangeMaterial(model.get(), m4.get(), m5);*/
}

void NextGenSonic::DisableGem(S06HUD_API::SonicGemType type)
{
    switch (type)
    {
    case S06HUD_API::SonicGemType::SGT_Red:
    {
        // Disable slowdown
        *(bool*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x19D }) = false;
        break;
    }
    case S06HUD_API::SonicGemType::SGT_White:
    {
        m_whiteGemEnabled = false;
        break;
    }
    case S06HUD_API::SonicGemType::SGT_Purple:
    {
        m_purpleGemEnabled = false;
        if (Common::GetPlayerModelScale() != 1.0f)
        {
            // Play increase size pfx
            static SharedPtrTypeless pfxHandle;
            void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
            Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle, matrixNode, "ef_ch_sng_purpleend", 1);
        }
        Common::SetPlayerModelScale(1.0f);

        // Revert air action code
        WRITE_MEMORY(0xDFFE76, uint8_t, 0x56, 0xE8, 0x94, 0x00, 0x06);
        break;
    }
    }
}

bool NextGenSonic::GetSkyGemHitLocation
(
    Eigen::Vector4f& o_pos, 
    Hedgehog::Math::CVector position, 
    Hedgehog::Math::CVector velocity, 
    float const simRate, 
    float const maxDist
)
{
    float distTravelled = 0.0f;
    while (distTravelled < maxDist)
    {
        // Do equation of motion manually
        Hedgehog::Math::CVector const velPrev = velocity;
        velocity += Hedgehog::Math::CVector::UnitY() * cSonic_skyGemGravity * simRate;
        Hedgehog::Math::CVector const posPrev = position;
        position += (velPrev + velocity) * 0.5f * simRate;

        distTravelled += (position - posPrev).norm();

        Eigen::Vector4f const rayStartPos(posPrev.x(), posPrev.y(), posPrev.z(), 1.0f);
        Eigen::Vector4f const rayEndPos(position.x(), position.y(), position.z(), 1.0f);
        Eigen::Vector4f outNormal;
        if (Common::fRaycast(rayStartPos, rayEndPos, o_pos, outNormal, *(uint32_t*)0x1E0AFB4))
        {
            return true;
        }
    }

    return false;
}

HOOK(int, __fastcall, NextGenSonicGems_MsgRestartStage, 0xE76810, Sonic::Player::CPlayer* player, void* Edx, void* message)
{
    NextGenSonic::ChangeGems(NextGenSonic::m_sonicGemType, S06HUD_API::SonicGemType::SGT_None);
    return originalNextGenSonicGems_MsgRestartStage(player, Edx, message);
}

HOOK(void, __fastcall, NextGenSonicGems_CSonicUpdate, 0xE6BF20, Sonic::Player::CPlayerSpeed* This, void* Edx, float* dt)
{
    // No gems for classic Sonic
    if (!*pModernSonicContext)
    {
        originalNextGenSonicGems_CSonicUpdate(This, Edx, dt);
        return;
    }

    // No gems for Super Sonic
    if (Common::IsPlayerSuper())
    {
        if (NextGenSonic::m_sonicGemType != S06HUD_API::SonicGemType::SGT_None)
        {
            // Force reset to no gem
            while (S06HUD_API::ScrollSonicGem(true, false) != S06HUD_API::SonicGemType::SGT_None) {}
            NextGenSonic::ChangeGems(NextGenSonic::m_sonicGemType, S06HUD_API::SonicGemType::SGT_None);
        }

        originalNextGenSonicGems_CSonicUpdate(This, Edx, dt);
        return;
    }

    CSonicStateFlags const* flags = Common::GetSonicStateFlags();
    Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
    if (!flags->OutOfControl)
    {
        if (padState->IsTapped(Sonic::EKeyState::eKeyState_DpadRight))
        {
            S06HUD_API::ScrollSonicGem(true, false);
        }
        else if (padState->IsTapped(Sonic::EKeyState::eKeyState_DpadLeft))
        {
            S06HUD_API::ScrollSonicGem(false, false);
        }
        else if (padState->IsTapped(Sonic::EKeyState::eKeyState_DpadUp))
        {
            while (S06HUD_API::ScrollSonicGem(true, false) != S06HUD_API::SonicGemType::SGT_None) {}
        }
    }

    S06HUD_API::SonicGemType newGemType = S06HUD_API::GetSonicGem();
    if (NextGenSonic::m_sonicGemType != newGemType)
    {
        // Play gem change sfx
        static SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 80041028, 1);

        NextGenSonic::ChangeGems(NextGenSonic::m_sonicGemType, newGemType);
    }

    if (!flags->OutOfControl && !flags->Dead)
    {
        bool isStateForbidden = StateManager::isCurrentAction(StateAction::ExternalControl)
                             || StateManager::isCurrentAction(StateAction::FinishExternalControlAir)
                             || StateManager::isCurrentAction(StateAction::TransformSp)
                             || Common::IsPlayerHangOn();
        float* boost = Common::GetPlayerBoost();
        switch (NextGenSonic::m_sonicGemType)
        {
        case S06HUD_API::SonicGemType::SGT_Red:
        {
            bool enabled = padState->IsDown(Sonic::EKeyState::eKeyState_RightTrigger) && *boost > 0.0f;

            // Enable slowdown
            *(bool*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x19D }) = enabled;

            // Drain gauge
            if (enabled)
            {
                *boost = max(0.0f, *boost - cSonic_gemDrainRate * *dt);
            }
            break;
        }
        case S06HUD_API::SonicGemType::SGT_Yellow:
        {
            if (!flags->InvokeThunderBarrier && padState->IsTapped(Sonic::EKeyState::eKeyState_RightTrigger) && *boost >= cSonic_yellowGemCost)
            {
                Common::SonicContextGetItemType(10); // ThunberBarrier
                *boost = max(0.0f, *boost - cSonic_yellowGemCost);
            }
            break;
        }
        case S06HUD_API::SonicGemType::SGT_Purple:
        {
            // Reset jump count
            if (Common::IsPlayerGrounded())
            {
                NextGenSonic::m_purpleGemJumpCount = 0;
            }

            NextGenSonic::m_purpleGemEnabled = padState->IsDown(Sonic::EKeyState::eKeyState_RightTrigger) && *boost > 0.0f 
                && !isStateForbidden && !Common::IsPlayerOnBoard();
            if (NextGenSonic::m_purpleGemEnabled)
            {
                // Drain gauge
                *boost = max(0.0f, *boost - cSonic_gemDrainRate * *dt);
                NextGenSonic::m_purpleGemBlockTimer = max(0.0f, NextGenSonic::m_purpleGemBlockTimer - *dt);

                if (Common::GetPlayerModelScale() != cSonic_purpleGemModelScale)
                {
                    // Override air action
                    WRITE_JUMP(0xDFFE76, NextGenSonicGems_PuepleGemAirAction);

                    // Scale Sonic
                    Common::SetPlayerModelScale(cSonic_purpleGemModelScale);

                    // Shrink sfx
                    static SharedPtrTypeless soundHandle;
                    Common::SonicContextPlaySound(soundHandle, 80041029, 1);

                    // Shrink pfx
                    static SharedPtrTypeless pfxHandle;
                    void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x10);
                    Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle, matrixNode, "ef_ch_sng_purplestart", 1);
                }
            }
            else
            {
                NextGenSonic::DisableGem(S06HUD_API::SonicGemType::SGT_Purple);
            }
            break;
        }
        case S06HUD_API::SonicGemType::SGT_Green:
        {
            if (!NextGenSonic::m_greenGemEnabled && !flags->KeepRunning && !Common::IsPlayerGrinding()
            && !isStateForbidden && !StateManager::isCurrentAction(StateAction::SquatKick)
            && padState->IsTapped(Sonic::EKeyState::eKeyState_RightTrigger) && *boost >= cSonic_greenGemCost)
            {
                // Hijack squat kick state
                NextGenSonic::m_greenGemEnabled = true;
                StateManager::ChangeState(StateAction::SquatKick, (void*)This->m_spContext.get());
                *boost = max(0.0f, *boost - cSonic_greenGemCost);
            }
            break;
        }
        case S06HUD_API::SonicGemType::SGT_Sky:
        {
            if (!padState->IsDown(Sonic::EKeyState::eKeyState_RightTrigger))
            {
                NextGenSonic::m_skyGemCancelled = false;
            }

            if (!NextGenSonic::m_skyGemEnabled && !flags->KeepRunning && Common::IsPlayerGrounded() && !Common::IsPlayerGrinding()
            && !NextGenSonic::m_skyGemCancelled && !isStateForbidden && !StateManager::isCurrentAction(StateAction::SquatKick)
            && padState->IsDown(Sonic::EKeyState::eKeyState_RightTrigger) && *boost >= cSonic_skyGemCost)
            {
                // Hijack squat kick state
                NextGenSonic::m_skyGemEnabled = true;
                StateManager::ChangeState(StateAction::SquatKick, (void*)This->m_spContext.get());
            }
            break;
        }
        }
    }
    else
    {
        NextGenSonic::DisableGem(NextGenSonic::m_sonicGemType);
    }

    originalNextGenSonicGems_CSonicUpdate(This, Edx, dt);
}

SharedPtrTypeless soundHandle_thunderShield;
HOOK(bool*, __fastcall, NextGenSonicGems_CPlayerSpeedStatePluginThunderBarrierBegin, 0x11DD970, void* This)
{
    // Play 06 yellow gem sfx (loops)
    Common::SonicContextPlaySound(soundHandle_thunderShield, 80041033, 1);

    // Disable original sound for modern
    if (*pModernSonicContext)
    {
        WRITE_MEMORY(0x11DD9A8, int, -1);
    }
    else
    {
        WRITE_MEMORY(0x11DD9A8, uint32_t, 6001008);
    }

    return originalNextGenSonicGems_CPlayerSpeedStatePluginThunderBarrierBegin(This);
}

HOOK(int, __fastcall, NextGenSonicGems_CPlayerSpeedStatePluginThunderBarrierEnd, 0x11DD900, void* This)
{
    soundHandle_thunderShield.reset();
    return originalNextGenSonicGems_CPlayerSpeedStatePluginThunderBarrierEnd(This);
}

HOOK(int, __stdcall, NextGenSonicGems_HomingUpdate, 0xE5FF10, Sonic::Player::CPlayerSpeedContext* context)
{
    return originalNextGenSonicGems_HomingUpdate(context);
}

FUNCTION_PTR(void, __thiscall, UpdateHomingCollision2D, 0xE642F0, Sonic::Player::CPlayerSpeedContext* This);
FUNCTION_PTR(void, __thiscall, UpdateHomingCollision3D, 0xE64410, Sonic::Player::CPlayerSpeedContext* This);
float HomingLockonCollisionFovy2DPrev = 0.0f;
float HomingLockonCollisionFovyN2DPrev = 0.0f;
float HomingLockonCollisionFovy3DPrev = 0.0f;
float HomingLockonCollisionFovyN3DPrev = 0.0f;
float* HomingLockonCollisionFovy2D = nullptr;
float* HomingLockonCollisionFovyN2D = nullptr;
float* HomingLockonCollisionFovy3D = nullptr;
float* HomingLockonCollisionFovyN3D = nullptr;
bool whiteGemHomingAttack = false;
HOOK(int, __fastcall, NextGenSonicGems_CSonicStateHomingAttackBegin, 0x1232040, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();

    // Play white gem charge
    if (NextGenSonic::m_whiteGemEnabled)
    {
        if (Common::fGetPlayerParameterPtr(EPlayerParameter::HomingLockonCollisionFar, *(void**)0x1E61C5C, HomingLockonCollisionFovy2D)
        && Common::fGetPlayerParameterPtr(EPlayerParameter::HomingLockonCollisionFarN, *(void**)0x1E61C5C, HomingLockonCollisionFovyN2D)
        && Common::fGetPlayerParameterPtr(EPlayerParameter::HomingLockonCollisionFar, *(void**)0x1E61C54, HomingLockonCollisionFovy3D)
        && Common::fGetPlayerParameterPtr(EPlayerParameter::HomingLockonCollisionFarN, *(void**)0x1E61C54, HomingLockonCollisionFovyN3D))
        {
            HomingLockonCollisionFovy2DPrev = *HomingLockonCollisionFovy2D;
            HomingLockonCollisionFovyN2DPrev = *HomingLockonCollisionFovyN2D;
            HomingLockonCollisionFovy3DPrev= *HomingLockonCollisionFovy3D;
            HomingLockonCollisionFovyN3DPrev = *HomingLockonCollisionFovyN3D;

            *HomingLockonCollisionFovy2D = cSonic_whiteGemHomingRange;
            *HomingLockonCollisionFovyN2D = cSonic_whiteGemHomingRange;
            *HomingLockonCollisionFovy3D = cSonic_whiteGemHomingRange;
            *HomingLockonCollisionFovyN3D = cSonic_whiteGemHomingRange;

            UpdateHomingCollision2D(context);
            UpdateHomingCollision3D(context);
        }

        static SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 80041025, 1);

        // Skip initial speed change, out of control
        WRITE_JUMP(0x1232083, (void*)0x1232450);

        // Skip initial rotation modification
        WRITE_JUMP(0x123248D, (void*)0x12324A7);

        // Skip locus effect
        WRITE_JUMP(0x1232508, (void*)0x1232511);

        // Need to set to prevent Sonic moving in Y-axis?
        *(uint32_t*)((uint32_t)context + 1968) = 0xCCBEBC20;
        Common::GetSonicStateFlags()->EnableHomingAttack = false;

        // Remember position
        NextGenSonic::m_gemHoldPosition = context->m_spMatrixNode->m_Transform.m_Position;

        // Use different Homing Attack After animation table
        WRITE_MEMORY(0x111838F, char const**, homingAttackAnim);

        // Increment NoDamage
        whiteGemHomingAttack = true;
        if (context->StateFlag(eStateFlag_NoDamage) != 0xFF)
        {
            context->StateFlag(eStateFlag_NoDamage)++;
        }
    }
    else
    {
        // Revert using original Homing Attack After animation table
        WRITE_MEMORY(0x111838F, uint32_t, 0x1E75E18);

        // Disable pointers
        HomingLockonCollisionFovy2D = nullptr;
        HomingLockonCollisionFovyN2D = nullptr;
        HomingLockonCollisionFovy3D = nullptr;
        HomingLockonCollisionFovyN3D = nullptr;
    }

    int result = originalNextGenSonicGems_CSonicStateHomingAttackBegin(This);

    // Revert original code
    WRITE_MEMORY(0x1232083, uint8_t, 0x83, 0xBB, 0x98, 0x0E, 0x00);
    WRITE_MEMORY(0x123248D, uint8_t, 0x0F, 0x28, 0x83, 0x90, 0x02);
    WRITE_MEMORY(0x1232508, uint8_t, 0x6A, 0x00, 0x8B, 0xC3, 0xE8);

    return result;
}

HOOK(void, __fastcall, NextGenSonicGems_CSonicStateHomingAttackAdvance, 0x1231C60, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    if (NextGenSonic::m_whiteGemEnabled)
    {
        Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
        if (padState->IsDown(Sonic::EKeyState::eKeyState_RightTrigger))
        {
            originalNextGenSonicGems_HomingUpdate(context);

            Common::SetPlayerVelocity(Eigen::Vector3f::Zero());
            Common::SetPlayerPosition(NextGenSonic::m_gemHoldPosition);
            This->m_Time = 0.0f;

            if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
            {
                // Cancel with B button
                StateManager::ChangeState(StateAction::Walk, context);
            }
            else if (!context->m_HomingAttackTargetActorID && padState->IsTapped(Sonic::EKeyState::eKeyState_A))
            {
                // Cancel with normal dummy homing attack
                StateManager::ChangeState(StateAction::HomingAttack, context);
            }
            else if (!context->m_Grounded && padState->IsTapped(Sonic::EKeyState::eKeyState_X))
            {
                // Cancel with bounce bracelet
                StateManager::ChangeState(StateAction::Stomping, context);
            }

            return;
        }
        else
        {
            NextGenSonic::m_whiteGemEnabled = false;

            // Release sfx/pfx
            static SharedPtrTypeless soundHandle;
            Common::SonicContextPlaySound(soundHandle, 80041026, 1);
            Common::SonicContextRequestLocusEffect();

            // Get homing target position and apply initial velocity
            if (context->m_HomingAttackTargetActorID)
            {
                context->m_pPlayer->SendMessage(context->m_HomingAttackTargetActorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&context->m_HomingAttackPosition));
                Eigen::Vector3f velocity = (context->m_HomingAttackPosition - context->m_spMatrixNode->m_Transform.m_Position).normalized() * cSonic_whiteGemSpeed;
                Common::SetPlayerVelocity(velocity);

                // Set OutOfControl
                FUNCTION_PTR(int, __stdcall, SetOutOfControl, 0xE5AC00, Sonic::Player::CPlayerSpeedContext * context, float duration);
                *(int*)((uint32_t)context + 0x80) = SetOutOfControl(context, -1.0f);
            }
            else
            {
                Eigen::Vector3f dir;
                Common::GetPlayerWorldDirection(dir, true);
                Common::SetPlayerVelocity(dir * cSonic_whiteGemDummySpeed);
            }

            // Update rotation
            Common::SonicContextUpdateRotationToVelocity(context, &context->m_Velocity, true);

            // Send MsgStartHomingChase message to homing target actor
            context->m_pPlayer->SendMessage(context->m_HomingAttackTargetActorID, boost::make_shared<Sonic::Message::MsgStartHomingChase>());

            // Deduct chaos energy
            context->m_ChaosEnergy = max(0.0f, context->m_ChaosEnergy - cSonic_whiteGemCost);

            // Don't stop by ground on homing attack
            WRITE_MEMORY(0x1231E36, uint8_t, 0xEB, 0x5E);
        }
    }

    originalNextGenSonicGems_CSonicStateHomingAttackAdvance(This);
}

HOOK(void, __fastcall, NextGenSonicGems_CSonicStateHomingAttackEnd, 0x1231F80, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();

    NextGenSonic::m_whiteGemEnabled = false;
    context->StateFlag(eStateFlag_OutOfControl) = false;

    // Decrement NoDamage
    if (whiteGemHomingAttack && context->StateFlag(eStateFlag_NoDamage) > 0)
    {
        context->StateFlag(eStateFlag_NoDamage)--;
    }
    whiteGemHomingAttack = false;

    // Revert stop by ground
    WRITE_MEMORY(0x1231E36, uint8_t, 0x74, 0x17);

    // Revert homing attack param
    if (HomingLockonCollisionFovy2D && HomingLockonCollisionFovyN2D && HomingLockonCollisionFovy3D && HomingLockonCollisionFovyN3D)
    {
        *HomingLockonCollisionFovy2D = HomingLockonCollisionFovy2DPrev;
        *HomingLockonCollisionFovyN2D = HomingLockonCollisionFovyN2DPrev;
        *HomingLockonCollisionFovy3D = HomingLockonCollisionFovy3DPrev;
        *HomingLockonCollisionFovyN3D = HomingLockonCollisionFovyN3DPrev;

        UpdateHomingCollision2D(context);
        UpdateHomingCollision3D(context);
    }

    originalNextGenSonicGems_CSonicStateHomingAttackEnd(This);
}

HOOK(void, __fastcall, NextGenSonicGems_CSonicStateHomingAttackAfterAdvance, 0x1118600, hh::fnd::CStateMachineBase::CStateBase* This)
{
    // Blue Gem as light attack
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    if (This->m_Time > cSonic_blueGemHomingDelay && context->m_ChaosEnergy >= cSonic_blueGemCost
    && NextGenSonic::m_sonicGemType == S06HUD_API::SonicGemType::SGT_Blue)
    {
        Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
        if (padState->IsDown(Sonic::EKeyState::eKeyState_RightTrigger))
        {
            originalNextGenSonicGems_HomingUpdate(context);
            if (context->m_HomingAttackTargetActorID)
            {
                context->m_ChaosEnergy -= cSonic_blueGemCost;
                StateManager::ChangeState(StateAction::HomingAttack, context);
                return;
            }
        }
    }

    originalNextGenSonicGems_CSonicStateHomingAttackAfterAdvance(This);
}

bool greenGemShockWaveCreated = false;
float greenGemShockWaveTimer = 0.0f;
SharedPtrTypeless skyGemSoundHandle;
bool skyGemThrowAnimationStarted = false;
bool skyGemThrowDummy = false;
HOOK(int*, __fastcall, NextGenSonicGems_CSonicStateSquatKickBegin, 0x12526D0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    if (NextGenSonic::m_greenGemEnabled)
    {
        greenGemShockWaveCreated = false;
        greenGemShockWaveTimer = 0.0f;

        // Change Sonic's animation
        Common::SonicContextChangeAnimation(context->m_Grounded ? AnimationSetPatcher::GreenGemGround : AnimationSetPatcher::GreenGemAir);

        // Tornado sfx
        static SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 80041027, 1);

        // Remember position (for air only)
        NextGenSonic::m_gemHoldPosition = context->m_spMatrixNode->m_Transform.m_Position;

        // Stop Sonic
        Common::GetSonicStateFlags()->OutOfControl = true;
        Common::SetPlayerVelocity(Eigen::Vector3f::Zero());

        return nullptr;
    }
    else if (NextGenSonic::m_skyGemEnabled)
    {
        NextGenSonic::m_skyGemLaunched = false;

        // Change Sonic's animation
        Common::SonicContextChangeAnimation(AnimationSetPatcher::SkyGem);

        // Sky gem charge sfx
        Common::SonicContextPlaySound(skyGemSoundHandle, 80041030, 1);

        // Stop Sonic
        Common::GetSonicStateFlags()->OutOfControl = true;
        Common::SetPlayerVelocity(Eigen::Vector3f::Zero());

        return nullptr;
    }

    return originalNextGenSonicGems_CSonicStateSquatKickBegin(This);
}

HOOK(void, __fastcall, NextGenSonicGems_CSonicStateSquatKickAdvance, 0x1252810, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    if (NextGenSonic::m_greenGemEnabled)
    {
        if (This->m_Time >= (context->m_Grounded ? 0.65f : 0.0f))
        {
            greenGemShockWaveTimer -= This->m_pStateMachine->m_UpdateInfo.DeltaTime;

            // Create shock wave per interval
            if (greenGemShockWaveTimer <= 0.0f)
            {
                greenGemShockWaveTimer = cSonic_greenGemShockWaveInterval;

                Hedgehog::Math::CVector pos = context->m_spMatrixNode->m_Transform.m_Position;
                pos.y() -= 2.0f;

                // After first one, create smaller shock waves to protect Sonic
                if (greenGemShockWaveCreated)
                {
                    Common::CreatePlayerSupportShockWave(pos, 4.0f, 4.0f, cSonic_greenGemShockWaveInterval);
                }
                else
                {
                    Common::CreatePlayerSupportShockWave(pos, cSonic_greenGemHeight, cSonic_greenGemRadius, 0.1f);
                }
            }

            // Tornado pfx
            if (!greenGemShockWaveCreated)
            {
                greenGemShockWaveCreated = true;

                static SharedPtrTypeless pfxHandle;
                void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x10);
                Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle, matrixNode, "ef_ch_sng_tornado", 1);
            }
        }

        if (context->m_Grounded)
        {
            // Remember position in case we become airborne
            NextGenSonic::m_gemHoldPosition = context->m_spMatrixNode->m_Transform.m_Position;
        }
        else
        {
            // Force stay in air
            Common::SetPlayerVelocity(Eigen::Vector3f::Zero());
            Common::SetPlayerPosition(NextGenSonic::m_gemHoldPosition);
        }

        if (This->m_Time > (context->m_Grounded ? 2.95f : 1.05f))
        {
            StateManager::ChangeState(context->m_Grounded ? StateAction::Walk : StateAction::Fall, context);
        }
        return;
    }
    else if (NextGenSonic::m_skyGemEnabled)
    {
        Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
        if (skyGemThrowAnimationStarted)
        {
            if (!NextGenSonic::m_skyGemLaunched && This->m_Time > 4.0f / 60.0f)
            {
                NextGenSonic::m_skyGemLaunched = true;
                skyGemSoundHandle.reset();

                // Deduct chaos energy
                context->m_ChaosEnergy = max(0.0f, context->m_ChaosEnergy - cSonic_skyGemCost);

                // Delete previous CObjSkyGem
                if (m_spSkyGemSingleton)
                {
                    ((CObjSkyGem*)m_spSkyGemSingleton.get())->Kill();
                }

                // Throw CObjSkyGem
                Hedgehog::Math::CVector position = context->m_spMatrixNode->m_Transform.m_Position + Hedgehog::Math::CVector::UnitY() * 1.0f;
                Hedgehog::Math::CVector playerRight = context->m_spMatrixNode->m_Transform.m_Rotation * Hedgehog::Math::CVector::UnitX();
                Hedgehog::Math::CVector playerFoward = context->m_spMatrixNode->m_Transform.m_Rotation * Hedgehog::Math::CVector::UnitZ();
                Hedgehog::Math::CVector direction = Eigen::AngleAxisf(CustomCamera::m_skyGemCameraPitch, playerRight) * context->m_spMatrixNode->m_Transform.m_Rotation * Hedgehog::Math::CVector::UnitZ();
                m_spSkyGemSingleton = boost::make_shared<CObjSkyGem>(position + playerFoward * 0.4f, direction, skyGemThrowDummy ? cSonic_skyGemLaunchSpeedDummy : cSonic_skyGemLaunchSpeed);
                context->m_pPlayer->m_pMember->m_pGameDocument->AddGameObject(m_spSkyGemSingleton);
            }

            if (This->m_Time > 20.0f / 60.0f)
            {
                StateManager::ChangeState(StateAction::Walk, context);
            }
        }
        else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B) || !context->m_Grounded || context->m_Velocity.norm() > 0.1f)
        {
            // Cancel with B button
            NextGenSonic::m_skyGemCancelled = true;
            StateManager::ChangeState(StateAction::Walk, context);
        }
        else if (!padState->IsDown(Sonic::EKeyState::eKeyState_RightTrigger))
        {
            skyGemThrowDummy = (This->m_Time <= cSonic_skyGemDummyThrowTime);
            if (skyGemThrowDummy)
            {
                CustomCamera::m_skyGemCameraEnabled = false;
            }

            if (This->m_Time > 16.0f / 60.0f)
            {
                // throw animation
                skyGemThrowAnimationStarted = true;
                Common::SonicContextChangeAnimation(AnimationSetPatcher::SkyGemEnd);

                This->m_Time = 0.0f;
            }
        }
        else
        {
            CustomCamera::m_skyGemCameraEnabled = true;
        }

        // Ignore dummy throw time
        if (CustomCamera::m_skyGemCameraEnabled && !NextGenSonic::m_skyGemLaunched && 
            ((!skyGemThrowAnimationStarted && This->m_Time > cSonic_skyGemDummyThrowTime) || skyGemThrowAnimationStarted && This->m_Time <= 4.0f / 60.0f))
        {
            Hedgehog::Math::CVector position = context->m_spMatrixNode->m_Transform.m_Position + Hedgehog::Math::CVector::UnitY() * 1.0f;
            Hedgehog::Math::CVector playerRight = context->m_spMatrixNode->m_Transform.m_Rotation * Hedgehog::Math::CVector::UnitX();
            Hedgehog::Math::CVector playerFoward = context->m_spMatrixNode->m_Transform.m_Rotation * Hedgehog::Math::CVector::UnitZ();
            Hedgehog::Math::CVector direction = Eigen::AngleAxisf(CustomCamera::m_skyGemCameraPitch, playerRight) * context->m_spMatrixNode->m_Transform.m_Rotation * Hedgehog::Math::CVector::UnitZ();

            Eigen::Vector4f lockonPos;
            if (NextGenSonic::GetSkyGemHitLocation(lockonPos, position + playerFoward * 0.4f, direction * cSonic_skyGemLaunchSpeed, 0.16666f, 100.0f))
            {
                // Spawn/Move lock-on cursor
                if (m_spSkyGemLockonCursor)
                {
                    ((CObjSkyGemLockonCursor*)m_spSkyGemLockonCursor.get())->SetPos(lockonPos);
                }
                else
                {
                    m_spSkyGemLockonCursor = boost::make_shared<CObjSkyGemLockonCursor>(lockonPos);
                    context->m_pPlayer->m_pMember->m_pGameDocument->AddGameObject(m_spSkyGemLockonCursor);
                }
            }
            else if (m_spSkyGemLockonCursor)
            {
                ((CObjSkyGemLockonCursor*)m_spSkyGemLockonCursor.get())->Kill();
            }
        }

        return;
    }

    originalNextGenSonicGems_CSonicStateSquatKickAdvance(This);
}

HOOK(int*, __fastcall, NextGenSonicGems_CSonicStateSquatKickEnd, 0x12527B0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    if (NextGenSonic::m_greenGemEnabled)
    {
        NextGenSonic::m_greenGemEnabled = false;
        Common::GetSonicStateFlags()->OutOfControl = false;
        return nullptr;
    }
    else if (NextGenSonic::m_skyGemEnabled)
    {
        // Remove lock-on cursor
        if (m_spSkyGemLockonCursor)
        {
            CObjSkyGemLockonCursor* cursor = ((CObjSkyGemLockonCursor*)m_spSkyGemLockonCursor.get());
            if (NextGenSonic::m_skyGemLaunched)
            {
                cursor->SetOutro();
            }
            else
            {
                cursor->Kill();
            }
        }

        CustomCamera::m_skyGemCameraEnabled = false;
        NextGenSonic::m_skyGemEnabled = false;
        NextGenSonic::m_skyGemLaunched = false;
        skyGemThrowAnimationStarted = false;
        Common::GetSonicStateFlags()->OutOfControl = false;
        skyGemSoundHandle.reset();
        return nullptr;
    }

    return originalNextGenSonicGems_CSonicStateSquatKickEnd(This);
}

//---------------------------------------------------
// Main Apply Patches
//---------------------------------------------------
void NextGenSonic::applyPatches()
{
    m_isElise = (Configuration::m_model == Configuration::ModelType::SonicElise);

    // Use new homing attack animation for Elise, set bounce bracelet status
    INSTALL_HOOK(NextGenSonic_CSonicStateHomingAttackBegin);

    if (!m_isElise)
    {
        // Always disable stomp voice and sfx for Sonic
        WRITE_MEMORY(0x1254E04, int, -1);
        WRITE_MEMORY(0x1254F23, int, -1);

        // Don't change to ball model during drift
        WRITE_NOP(0xDF30AB, 0xD);

        if (Configuration::m_unlimitedGauge)
        {
            INSTALL_HOOK(NextGenSonic_CHudSonicStageUpdate);
        }
    }

    // Fix fire/electric damage bone location
    WRITE_MEMORY(0x1ABD060, char*, "TopHair1"); // Needle_U_C
    WRITE_MEMORY(0x1ABD064, char*, "LeftHand"); // Hand_L
    WRITE_MEMORY(0x1ABD068, char*, "RightHand"); // Hand_R
    WRITE_MEMORY(0x1ABD06C, char*, "LeftToeBase"); // Toe_L
    WRITE_MEMORY(0x1ABD070, char*, "RightToeBase"); // Toe_R
    WRITE_MEMORY(0x1203CA3, uint32_t, 0x1ABD05C); // electic damage use fire damage offsets
    WRITE_MEMORY(0x1203D7C, uint32_t, 0x1ABD074);

    // Fix Super Sonic pfx bone location
    static float const spineAuraOffset = -50.0f;
    WRITE_MEMORY(0xDA2689, float*, &spineAuraOffset);
    WRITE_MEMORY(0xDA26EA, char*, "TopHair1"); // Needle_U_C
    WRITE_MEMORY(0xDA273B, char*, "LeftHand"); // Hand_L
    WRITE_MEMORY(0xDA278C, char*, "RightHand"); // Hand_R
    WRITE_MEMORY(0xDA27DD, char*, "LeftFoot"); // Foot_L
    WRITE_MEMORY(0xDA285E, char*, "RightFoot"); // Foot_R

    if (!Configuration::m_characterMoveset) return;

    //-------------------------------------------------------
    // X-Action State handling
    //-------------------------------------------------------
    {
        // Return 0 for Squat and Sliding, handle them ourselves
        WRITE_MEMORY(0xDFF8D5, uint8_t, 0xEB, 0x05);
        WRITE_MEMORY(0xDFF856, uint8_t, 0xE9, 0x81, 0x00, 0x00, 0x00);
        INSTALL_HOOK(NextGenSonic_BActionHandler);
        INSTALL_HOOK(NextGenSonic_MsgRestartStage);
        INSTALL_HOOK(NextGenSonic_MsgNotifyObjectEvent);
    }

    //-------------------------------------------------------
    // Brake Flip
    //-------------------------------------------------------
    if (!m_isElise)
    {
        INSTALL_HOOK(NextGenSonic_CSonicStateSlidingEndBegin);
        INSTALL_HOOK(NextGenSonic_CSonicStateSlidingEndAdvance);
        INSTALL_HOOK(NextGenSonic_CSonicStateSlidingEndEnd);
    }

    //-------------------------------------------------------
    // Bounce Bracelet
    //-------------------------------------------------------
    if (!m_isElise)
    {
        INSTALL_HOOK(NextGenSonic_CSonicStateStompingBegin);
        WRITE_JUMP(0x1254A36, bounceBraceletASMImpl);
        WRITE_JUMP(0x12549C9, bounceBraceletASMImpl);

        // Re-enable jumpball collision and play pfx
        INSTALL_HOOK(NextGenSonic_CSonicStateJumpBallBegin);

        // Replace stomp land sfx with bounce
        WRITE_MEMORY(0x12548FC, uint32_t, 80041022);
        WRITE_MEMORY(0x12549D8, uint32_t, 80041022);

        // Allow spin attack OnWater
        WRITE_MEMORY(0x12352B8, uint8_t, 0xEB);

        // Skip B action check on Stomping
        WRITE_NOP(0x12549CF, 0x8);
        WRITE_NOP(0x12549DC, 0x2);
        WRITE_MEMORY(0x12549DE, uint8_t, 0xEB);

        // Change jumpball to hit enemy as if you're squat kicking (TypeSonicSquatKick)
        WRITE_MEMORY(0x11BCC43, uint32_t, SonicCollision::TypeSonicSquatKick); // jumpball start
        WRITE_MEMORY(0x11BCBB2, uint32_t, SonicCollision::TypeSonicSquatKick); // jumpball end

        // Set to custom bounce pfx from 06
        WRITE_MEMORY(0x1249C32, char**, &ef_ch_sng_bound);
        WRITE_MEMORY(0x1254E7B, uint8_t, 0x30);
        WRITE_MEMORY(0x1254E9A, char**, &ef_ch_sng_bound_down);
        WRITE_MEMORY(0x1249B39, char**, &ef_ch_sng_bound_strong);
        WRITE_MEMORY(0x1249C19, char**, &ef_ch_sps_bound);
        WRITE_MEMORY(0x1254E57, uint8_t, 0x30);
        WRITE_MEMORY(0x1254E73, char**, &ef_ch_sps_bound_down);

        // Allow bounce bracelet immediately after homing attack
        INSTALL_HOOK(NextGenSonic_CSonicStateHomingAttackAfterAdvance);
    }

    //-------------------------------------------------------
    // Sweep Kick
    //-------------------------------------------------------
    if (!m_isElise)
    {
        INSTALL_HOOK(NextGenSonic_CSonicStateSquatKickBegin);
        INSTALL_HOOK(NextGenSonic_CSonicStateSquatKickAdvance);
        WRITE_JUMP(0x125291F, NextGenSonic_CSonicStateSquatKickAdvanceTransitionOut);
        INSTALL_HOOK(NextGenSonic_CSonicStateSquatKickEnd);

        // Don't transition out to Stand, only Walk and Fall
        WRITE_MEMORY(0x1252905, uint32_t, 0x15F4FE8);

        // Enable sweep kick attack collision immediately
        static double const c_sweepKickActivateTime = 0.0;
        WRITE_MEMORY(0x125299E, double*, &c_sweepKickActivateTime);

        // Change SquatKick's collision the same as sliding
        WRITE_MEMORY(0xDFCD6D, uint8_t, 0x5); // switch 6 cases
        static uint32_t const collisionSwitchTable[6] =
        {
            0xDFCD7B, // normal
            0xDFCDC0, // slide
            0xDFCD7B, // boost
            0xDFCD7B,
            0xDFCDFA, // unused
            0xDFCDC0  // squat kick
        };
        WRITE_MEMORY(0xDFCD77, uint32_t*, collisionSwitchTable);
    }

    //-------------------------------------------------------
    // Anti-Gravity
    //-------------------------------------------------------
    {
        // Change slide to hit enemy as if you're squat kicking (TypeSonicSquatKick)
        WRITE_MEMORY(0x11D72F3, uint32_t, SonicCollision::TypeSonicSquatKick);
        WRITE_MEMORY(0x11D7090, uint32_t, SonicCollision::TypeSonicSquatKick);

        // Disable all Sliding transition out, handle them outselves
        WRITE_MEMORY(0x11D6B7D, uint8_t, 0xEB);
        WRITE_MEMORY(0x11D6CA2, uint8_t, 0xEB);
        WRITE_MEMORY(0x11D6F82, uint8_t, 0x90, 0xE9);

        // Change sliding animation if we are spindashing, handle transition out
        INSTALL_HOOK(NextGenSonic_CSonicStateSlidingBegin);
        INSTALL_HOOK(NextGenSonic_CSonicStateSlidingAdvance);
        WRITE_JUMP(0x11D7027, NextGenSonic_CSonicStateSlidingEnd);

        // Set constant sliding speed
        WRITE_JUMP(0x11D989B, NextGenSonic_slidingHorizontalTargetVel2D);
        WRITE_JUMP(0x11D98A7, NextGenSonic_slidingHorizontalTargetVel2D);
        WRITE_JUMP(0x11D9532, NextGenSonic_slidingHorizontalTargetVel3D);

        // Do not use sliding fail pfx
        WRITE_NOP(0x11D6A6D, 2);

        // Increase turn rate for Spindash and Anti-Gravity
        static float slideTurnRate = 100.0f;
        WRITE_MEMORY(0x11D9441, float*, &slideTurnRate);
    }

    //-------------------------------------------------------
    // Spindashing
    //-------------------------------------------------------
    if (!m_isElise)
    {
        // Spin animation for Squat
        WRITE_MEMORY(0x1230A85, uint32_t, 0x15F84F4); // slide begin animation
        WRITE_MEMORY(0x1230A9F, uint32_t, 0x15F84F4); // slide begin animation
        WRITE_MEMORY(0x1230D74, uint32_t, 0x15F84F4); // slide hold animation

        // Use spindash when release button
        WRITE_JUMP(0x1230BDB, startSpindash);
        WRITE_MEMORY(0x1230C3A, uint32_t, 0x15F5108); // change state to sliding

        // If in tight spaces, still allow Sonic to unduck (aka use spindash)
        WRITE_NOP(0x1230BCB, 0x2);

        // Don't allow stick move start sliding from squat
        WRITE_MEMORY(0x1230D62, uint8_t, 0xEB);
        WRITE_MEMORY(0x1230DA9, uint8_t, 0xE9, 0xA8, 0x00, 0x00, 0x00, 0x90);

        // Play spindash sfx
        INSTALL_HOOK(NextGenSonic_CSonicStateSquatBegin);
        INSTALL_HOOK(NextGenSonic_CSonicStateSquatAdvance);
        INSTALL_HOOK(NextGenSonic_CSonicStateSquatEnd);
    }

    //-------------------------------------------------------
    // Rechargable Shield
    //-------------------------------------------------------
    if (m_isElise)
    {
        // Don't add boost from rings
        WRITE_MEMORY(0xE6853B, uint8_t, 0xEB);

        // Disable boost for normal Sonic only
        WRITE_JUMP(0xDFF268, NextGenSonic_groundBoostSuperSonicOnly);
        WRITE_JUMP(0xDFE05F, NextGenSonic_airBoostSuperSonicOnly);

        // Disable boost inside Perfect Chaos
        WRITE_MEMORY(0x11A06F7, uint8_t, 0xE9, 0xC1, 0x00, 0x00, 0x00, 0x90);

        // Handle shield and gauge
        INSTALL_HOOK(NextGenSonic_CSonicUpdateEliseShield);

        // Hijack boost plugin as shield
        INSTALL_HOOK(NextGenSonic_CSonicStatePluginBoostBegin);
        WRITE_JUMP(0x1117728, NextGenSonic_CSonicStatePluginBoostAdvance);
        INSTALL_HOOK(NextGenSonic_CSonicStatePluginBoostEnd);

        //WRITE_JUMP(0xE303F3, (void*)0xE30403); // boost forward push

        // Change "Stomping" type object physics to "Normal"
        WRITE_MEMORY(0xE9FFC9, uint32_t, 5);

        // Fix Homing Attack getting locked after using it once on water
        WRITE_NOP(0x119C932, 7);
    }
}

void NextGenSonic::applyPatchesPostInit()
{
    // Handle model hide/unhide
    INSTALL_HOOK(NextGenSonicGems_MsgRestartStage);

    if (!Configuration::m_characterMoveset) return;

    //-------------------------------------------------------
    // Gems
    //-------------------------------------------------------
    if (!m_isElise && Configuration::m_gemsEnabled)
    {
        INSTALL_HOOK(NextGenSonicGems_CSonicUpdate);

        // Ignore D-pad input for Sonic's control
        WRITE_JUMP(0xD97B56, (void*)0xD97B9E);

        // Never for land if staying in air for >1s
        WRITE_MEMORY(0xE33A72, uint8_t, 0xE9, 0x9D, 0x00, 0x00, 0x00);

        // Yellow Gem
        INSTALL_HOOK(NextGenSonicGems_CPlayerSpeedStatePluginThunderBarrierBegin);
        INSTALL_HOOK(NextGenSonicGems_CPlayerSpeedStatePluginThunderBarrierEnd);

        // White Gem
        INSTALL_HOOK(NextGenSonicGems_CSonicStateHomingAttackBegin);
        INSTALL_HOOK(NextGenSonicGems_CSonicStateHomingAttackAdvance);
        INSTALL_HOOK(NextGenSonicGems_CSonicStateHomingAttackEnd);

        // Green Gem
        INSTALL_HOOK(NextGenSonicGems_CSonicStateSquatKickBegin);
        INSTALL_HOOK(NextGenSonicGems_CSonicStateSquatKickAdvance);
        INSTALL_HOOK(NextGenSonicGems_CSonicStateSquatKickEnd);

        // Blue Gem
        INSTALL_HOOK(NextGenSonicGems_CSonicStateHomingAttackAfterAdvance);
    }
}
