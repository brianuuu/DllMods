#include "NextGenSonic.h"
#include "Configuration.h"
#include "Application.h"
#include "AnimationSetPatcher.h"

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
float const c_bouncePower = 18.0f;
float const c_bouncePowerBig = 23.0f;

bool NextGenSonic::m_isSquatKick = false;
bool NextGenSonic::m_isBrakeFlip = false;
Eigen::Vector3f NextGenSonic::m_brakeFlipDir(0, 0, 1);
float NextGenSonic::m_squatKickSpeed = 0.0f;
float const c_squatKickPressMaxTime = 0.3f;

bool slidingEndWasSliding = false;
bool NextGenSonic::m_isSpindash = false;
bool NextGenSonic::m_isSliding = false;
float NextGenSonic::m_slidingTime = 0.0f;
float NextGenSonic::m_slidingSpeed = 0.0f;
float const c_slidingTime = 3.0f;
float const c_slidingSpeedMin = 10.0f;
float const c_slidingSpeedMax = 16.0f;
float const c_spindashTime = 3.0f;
float const c_spindashSpeed = 30.0f;

float NextGenSonic::m_bHeldTimer = 0.0f;

char const* ef_ch_sng_bound = "ef_ch_sng_bound";
char const* ef_ch_sps_bound = "ef_ch_sps_bound";
char const* ef_ch_sng_bound_down = "ef_ch_sng_bound_down";
char const* ef_ch_sps_bound_down = "ef_ch_sps_bound_down";
char const* ef_ch_sng_bound_strong = "ef_ch_sng_bound_strong";
char const* ef_ch_sng_spindash = "ef_ch_sng_spindash";

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
// CSonicStateFall
//---------------------------------------------------
uint8_t pendingFallAnimation_Sonic = 0;
HOOK(int, __fastcall, NextGenSonic_CSonicStateFallBegin, 0x1118FB0, void* This)
{
    alignas(16) MsgGetAnimationInfo message {};
    Common::SonicContextGetAnimationInfo(message);
    //printf("Animation = %s\n", message.m_name);

    if (message.IsAnimation("TrickPrepare") ||
        message.IsAnimation("UpReelEnd") ||
        message.IsAnimation("LookBack") ||
        message.IsAnimation("DashRingL") ||
        message.IsAnimation("DashRingR") ||
        message.IsAnimation("JumpSpring") ||
        message.IsAnimation("JumpBoard") ||
        message.IsAnimation("JumpBoardRev") ||
        message.IsAnimation("JumpBoardSpecialL") ||
        message.IsAnimation("JumpBoardSpecialR"))
    {
        // Delay transition to SpinFall until we start falling
        pendingFallAnimation_Sonic = message.IsAnimation("TrickPrepare") ? 2 : 1;
        WRITE_JUMP(0x111911F, (void*)0x1119188);
    }
    else
    {
        pendingFallAnimation_Sonic = false;
        WRITE_MEMORY(0x111911F, uint8_t, 0x8D, 0x8E, 0xA0, 0x02, 0x00, 0x00);
    }

    return originalNextGenSonic_CSonicStateFallBegin(This);
}

HOOK(bool, __fastcall, NextGenSonic_CSonicStateFallAdvance, 0x1118C50, void* This)
{
    switch (pendingFallAnimation_Sonic)
    {
    case 2:
    {
        // Trick animation
        alignas(16) MsgGetAnimationInfo message {};
        Common::SonicContextGetAnimationInfo(message);

        if (message.m_frame >= 92.0f)
        {
            Common::SonicContextChangeAnimation("Fall");
            pendingFallAnimation_Sonic = false;
        }

        break;
    }
    case 1:
    {
        // Other animations
        Eigen::Vector3f playerVelocity;
        Common::GetPlayerVelocity(playerVelocity);
        float const vSpeed = playerVelocity.y();
        playerVelocity.y() = 0.0f;
        float const hSpeed = playerVelocity.norm();

        if (Common::IsPlayerSuper())
        {
            if (vSpeed <= 0.0f)
            {
                Common::SonicContextChangeAnimation("Fall");
                pendingFallAnimation_Sonic = false;
            }
        }
        else if (vSpeed < 5.0f)
        {
            Common::SonicContextChangeAnimation(hSpeed <= 5.0f ? AnimationSetPatcher::SpinFallSpring : AnimationSetPatcher::SpinFall);
            pendingFallAnimation_Sonic = false;
        }

        break;
    }
    default: break;
    }

    return originalNextGenSonic_CSonicStateFallAdvance(This);
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
            NextGenSonic::m_slidingSpeed = max(NextGenSonic::m_slidingSpeed, c_slidingSpeedMin);
            NextGenSonic::m_slidingSpeed = min(NextGenSonic::m_slidingSpeed, c_slidingSpeedMax);
        }
    }

    NextGenSonic::m_slidingTime = NextGenSonic::m_isSpindash ? c_spindashTime : c_slidingTime;
    return originalNextGenSonic_CSonicStateSlidingBegin(This);
}

HOOK(void, __fastcall, NextGenSonic_CSonicStateSlidingAdvance, 0x11D69A0, void* This)
{
    originalNextGenSonic_CSonicStateSlidingAdvance(This);

    bool bDown, bPressed, bReleased;
    NextGenSonic::getActionButtonStates(bDown, bPressed, bReleased);
    NextGenSonic::m_slidingTime -= Application::getDeltaTime();
    if (bPressed || NextGenSonic::m_slidingTime <= 0.0f)
    {
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
    float minSpeed = (NextGenSonic::m_isSpindash ? c_spindashSpeed : c_slidingSpeedMin) - 5.0f;
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

    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        // About to transition out and on ground
        CSonicStateFlags* flags = Common::GetSonicStateFlags();
        if (!flags->KeepRunning
            && !StateManager::isCurrentAction(StateAction::SquatKick)
            && !StateManager::isCurrentAction(StateAction::Fall))
        {
            Eigen::Vector3f inputDirection;
            if (Common::GetWorldInputDirection(inputDirection) && inputDirection.isZero())
            {
                // Stops Sonic completely if no stick input
                slidingEndWasSliding = NextGenSonic::m_isSliding;
                StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
            }
        }
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
// B/X Button Action
//---------------------------------------------------
HOOK(bool, __stdcall, NextGenSonic_BActionHandler, 0xDFF660, CSonicContext* context, bool buttonHoldCheck)
{
    // Reset bounce bracelet status
    if (!NextGenSonic::m_isStomping)
    {
        NextGenSonic::m_bounced = false;
    }
    NextGenSonic::m_isStomping = false;

    // Handle B/X button action
    bool result = originalNextGenSonic_BActionHandler(context, buttonHoldCheck);
    if (result || Common::IsPlayerControlLocked())
    {
        NextGenSonic::m_bHeldTimer = 0.0f;
        return result;
    }

    return NextGenSonic::bActionHandlerImpl();
}

void NextGenSonic::getActionButtonStates(bool& bDown, bool& bPressed, bool& bReleased)
{
    Sonic::SPadState* padState = Sonic::CInputState::GetPadState();
    Sonic::EKeyState actionButton =
        Configuration::m_xButtonAction ?
        Sonic::EKeyState::eKeyState_X :
        Sonic::EKeyState::eKeyState_B;

    bDown = padState->IsDown(actionButton);
    bPressed = padState->IsTapped(actionButton);

    // Release button doesn't work for keyboard, get from Application.h
    bReleased = Application::getKeyIsReleased(actionButton);
    //bReleased = padState->IsReleased(actionButton);
}

bool NextGenSonic::bActionHandlerImpl()
{
    Eigen::Vector3f playerVelocity;
    if (!Common::GetPlayerVelocity(playerVelocity))
    {
        return false;
    }

    CSonicStateFlags* flags = Common::GetSonicStateFlags();
    bool moving = playerVelocity.norm() > 0.2f;
    bool canUseSpindash = !moving || (Configuration::m_rapidSpindash && !flags->KeepRunning);

    bool bDown, bPressed, bReleased;
    getActionButtonStates(bDown, bPressed, bReleased);
    if (bDown)
    {
        // Standing still and held B for a while (Spin Dash)
        if (canUseSpindash && m_bHeldTimer > c_squatKickPressMaxTime)
        {
            if (!m_isElise)
            {
                StateManager::ChangeState(StateAction::Squat, *PLAYER_CONTEXT);
                m_bHeldTimer = 0.0f;
                return true;
            }
        }

        // Remember how long we held B
        // NOTE: Apparently this code always runs at 60fps
        // If 30fps this will run twice per frame!
        m_bHeldTimer += 1.0f / 60.0f;
    }
    else
    {
        if (bReleased && !flags->OnWater)
        {
            if (m_bHeldTimer <= c_squatKickPressMaxTime)
            {
                if (!m_isElise)
                {
                    // Release B without holding it for too long (Squat Kick)
                    StateManager::ChangeState(StateAction::SquatKick, *PLAYER_CONTEXT);
                    m_bHeldTimer = 0.0f;
                    return true;
                }
            }
            else if (moving && !flags->KeepRunning && m_bHeldTimer > c_squatKickPressMaxTime)
            {
                if (!m_isElise && Configuration::m_rapidSpindash)
                {
                    // Disable anti-gravity if rapid spindash is enabled
                }
                else
                {
                    // Moving and released B (Anti-Gravity)
                    StateManager::ChangeState(StateAction::Sliding, *PLAYER_CONTEXT);
                    m_bHeldTimer = 0.0f;
                    return true;
                }
            }
        }

        m_bHeldTimer = 0.0f;
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
    velocity[1] = NextGenSonic::m_bounced ? c_bouncePowerBig : c_bouncePower;
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
        message.m_impulse *= c_spindashSpeed;
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
        playerDir *= c_spindashSpeed;

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

        //-------------------------------------------------------
        // Spin Fall
        //-------------------------------------------------------
        // Transition to Fall immediately without checking down speed for CPlayerSpeedStateSpecialJump
        WRITE_JUMP(0x11DE320, (void*)0x11DE344);

        // Don't transition to FallLarge when speed < -15.0f
        WRITE_MEMORY(0x1118DE5, uint8_t, 0xEB);

        // Change animation at CSonicStateFall base on current animation
        INSTALL_HOOK(NextGenSonic_CSonicStateFallBegin);
        INSTALL_HOOK(NextGenSonic_CSonicStateFallAdvance);
    }

    if (!Configuration::m_characterMoveset) return;

    //-------------------------------------------------------
    // B-Action State handling
    //-------------------------------------------------------
    {
        // Return 0 for Squat and Sliding, handle them ourselves
        WRITE_MEMORY(0xDFF8D5, uint8_t, 0xEB, 0x05);
        WRITE_MEMORY(0xDFF856, uint8_t, 0xE9, 0x81, 0x00, 0x00, 0x00);
        INSTALL_HOOK(NextGenSonic_BActionHandler);
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
    }

    //-------------------------------------------------------
    // Sweep Kick
    //-------------------------------------------------------
    if (!m_isElise)
    {
        // Play squat kick sfx
        INSTALL_HOOK(NextGenSonic_CSonicStateSquatKickBegin);
        INSTALL_HOOK(NextGenSonic_CSonicStateSquatKickAdvance);
        INSTALL_HOOK(NextGenSonic_CSonicStateSquatKickEnd);
        if (Configuration::m_language == Configuration::LanguageType::English)
        {
            WRITE_MEMORY(0x1252740, uint32_t, 3002020);
        }
        else
        {
            // Don't play voice for Japanese Sonic
            WRITE_NOP(0x1252731, 2);
            WRITE_JUMP(0x1252737, (void*)0x125277F);
        }

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
}
