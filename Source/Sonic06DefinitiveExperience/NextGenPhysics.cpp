#include "NextGenPhysics.h"
#include "Configuration.h"
#include "Application.h"
#include "AnimationSetPatcher.h"

float const c_funcMaxTurnRate = 400.0f;
float const c_funcTurnRateMultiplier = PI_F * 10.0f;
float NextGenPhysics::m_homingDownSpeed = 0.0f;
float const c_homingDownSpeedAdd = 15.0f;

bool NextGenPhysics::m_isBrakeFlip = false;

bool NextGenPhysics::m_isStomping = false;
bool NextGenPhysics::m_bounced = false;
float const c_bouncePower = 18.0f;
float const c_bouncePowerBig = 23.0f;

bool NextGenPhysics::m_isSquatKick = false;
Eigen::Vector3f NextGenPhysics::m_brakeFlipDir(0, 0, 1);
float NextGenPhysics::m_squatKickSpeed = 0.0f;
float const c_squatKickPressMaxTime = 0.3f;

bool slidingEndWasSliding = false;
bool NextGenPhysics::m_isSpindash = false;
bool NextGenPhysics::m_isSliding = false;
float NextGenPhysics::m_slidingTime = 0.0f;
float const c_slidingTime = 3.0f;
float const c_slidingSpeedMin = 10.0f;
float const c_slidingSpeedMax = 16.0f;
float const c_spindashTime = 3.0f;
float const c_spindashSpeed = 30.0f;

float NextGenPhysics::m_bHeldTimer = 0.0f;

HOOK(void, __stdcall, CSonicRotationAdvance, 0xE310A0, void* a1, float* targetDir, float turnRate1, float turnRateMultiplier, bool noLockDirection, float turnRate2)
{
    if (noLockDirection && !Common::IsPlayerOnBoard())
    {
        // If direction is not locked, pump up turn rate
        originalCSonicRotationAdvance(a1, targetDir, c_funcMaxTurnRate, c_funcTurnRateMultiplier, noLockDirection, c_funcMaxTurnRate);
    }
    else
    {
        originalCSonicRotationAdvance(a1, targetDir, turnRate1, turnRateMultiplier, noLockDirection, turnRate2);
    }
}

HOOK(char, __stdcall, CSonicStateGrounded, 0xDFF660, int* a1, bool a2)
{
    if (!NextGenPhysics::m_isStomping)
    {
        NextGenPhysics::m_bounced = false;
    }
    NextGenPhysics::m_isStomping = false;
    return originalCSonicStateGrounded(a1, a2);
}

bool pendingFallAnimation = false;
HOOK(int, __fastcall, CSonicStateFallBegin, 0x1118FB0, void* This)
{
    alignas(16) MsgGetAnimationInfo message {};
    Common::SonicContextGetAnimationInfo(message);
    //printf("Animation = %s\n", message.m_name);

    if (message.IsAnimation("UpReelEnd") ||
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
        pendingFallAnimation = true;
        WRITE_JUMP(0x111911F, (void*)0x1119188);
    }
    else
    {
        pendingFallAnimation = false;
        WRITE_MEMORY(0x111911F, uint8_t, 0x8D, 0x8E, 0xA0, 0x02, 0x00, 0x00);
    }

    return originalCSonicStateFallBegin(This);
}

HOOK(bool, __fastcall, CSonicStateFallAdvance, 0x1118C50, void* This)
{
    if (pendingFallAnimation)
    {
        Eigen::Vector3f playerVelocity;
        Common::GetPlayerVelocity(playerVelocity);
        float const vSpeed = playerVelocity.y();
        playerVelocity.y() = 0.0f;
        float const hSpeed = playerVelocity.norm();

        if (vSpeed < 5.0f)
        {
            //printf("hSpeed = %.3f\n", hSpeed);
            Common::SonicContextChangeAnimation(hSpeed <= 5.0f ? AnimationSetPatcher::SpinFallSpring : AnimationSetPatcher::SpinFall);
            pendingFallAnimation = false;
        }
    }

    return originalCSonicStateFallAdvance(This);
}

HOOK(int, __fastcall, CSonicStateStompingBegin, 0x1254CA0, void* This)
{
    NextGenPhysics::m_isStomping = true;
    return originalCSonicStateStompingBegin(This);
}

HOOK(int, __fastcall, CSonicStateHomingAttackBegin, 0x1232040, void* This)
{
    // Remember down speed just before homing attack
    if (Configuration::m_physics)
    {
        Eigen::Vector3f playerVelocity;
        Common::GetPlayerVelocity(playerVelocity);
        NextGenPhysics::m_homingDownSpeed = min(playerVelocity.y(), 0.0f);
        //printf("Down speed = %.3f\n", NextGenPhysics::m_homingDownSpeed);
    }

    // For Sonic's bounce bracelet
    NextGenPhysics::m_bounced = false;

    return originalCSonicStateHomingAttackBegin(This);
}

HOOK(int*, __fastcall, CSonicStateHomingAttackEnd, 0x1231F80, void* This)
{
    // Apply down speed before homing attack
    if (Configuration::m_physics && StateManager::isCurrentAction(StateAction::Fall))
    {
        Eigen::Vector3f playerVelocity;
        Common::GetPlayerVelocity(playerVelocity);
        playerVelocity.y() += NextGenPhysics::m_homingDownSpeed - c_homingDownSpeedAdd;
        Common::SetPlayerVelocity(playerVelocity);
    }

    return originalCSonicStateHomingAttackEnd(This);
}

HOOK(char*, __fastcall, CSonicStateJumpBallBegin, 0x11BCBE0, void* This)
{
    if (!NextGenPhysics::m_bounced)
    {
        // Disable jumpball collision
        WRITE_MEMORY(0x11BCC4B, uint8_t, 0x71);
    }
    else
    {
        WRITE_MEMORY(0x11BCC4B, uint8_t, 0xC1);
    }

    return originalCSonicStateJumpBallBegin(This);
}

HOOK(int*, __fastcall, CSonicStateSquatKickBegin, 0x12526D0, void* This)
{
    // Don't allow direction change for squat kick
    WRITE_MEMORY(0x11D944A, uint8_t, 0);

    // Get initial brake flip direction
    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (Common::GetPlayerTransform(playerPosition, playerRotation))
    {
        NextGenPhysics::m_brakeFlipDir = playerRotation * Eigen::Vector3f::UnitZ();
    }

    // Get current speed so we can keep it
    Eigen::Vector3f playerVelocity;
    if (Common::GetPlayerVelocity(playerVelocity))
    {
        playerVelocity.y() = 0.0f;
        NextGenPhysics::m_squatKickSpeed = playerVelocity.norm();
    }

    // Play squat kick sfx
    static SharedPtrTypeless soundHandle;
    Common::SonicContextPlaySound(soundHandle, 80041021, 1);

    NextGenPhysics::m_isSquatKick = true;
    return originalCSonicStateSquatKickBegin(This);
}

HOOK(void, __fastcall, CSonicStateSquatKickAdvance, 0x1252810, void* This)
{
    originalCSonicStateSquatKickAdvance(This);

    // Lock squat kick's rotation if not moving
    if (NextGenPhysics::m_squatKickSpeed == 0.0f)
    {
        alignas(16) float dir[4] = { NextGenPhysics::m_brakeFlipDir.x(), NextGenPhysics::m_brakeFlipDir.y(), NextGenPhysics::m_brakeFlipDir.z(), 0 };
        originalCSonicRotationAdvance(This, dir, c_funcMaxTurnRate, c_funcTurnRateMultiplier, true, c_funcMaxTurnRate);
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
                slidingEndWasSliding = NextGenPhysics::m_isSliding;
                StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
            }
        }
    }
}

HOOK(int*, __fastcall, CSonicStateSquatKickEnd, 0x12527B0, void* This)
{
    // Unlock direction change for sliding/spindash
    WRITE_MEMORY(0x11D944A, uint8_t, 1);

    NextGenPhysics::m_isSquatKick = false;
    return originalCSonicStateSquatKickEnd(This);
}

static SharedPtrTypeless spinDashSoundHandle;
HOOK(int*, __fastcall, CSonicStateSquatBegin, 0x1230A30, void* This)
{
    // Play spindash charge sfx
    Common::SonicContextPlaySound(spinDashSoundHandle, 2002042, 1);
    return originalCSonicStateSquatBegin(This);
}

HOOK(void, __fastcall, CSonicStateSquatAdvance, 0x1230B60, void* This)
{
    originalCSonicStateSquatAdvance(This);
    Eigen::Vector3f worldDirection;
    if (!Common::GetPlayerWorldDirection(worldDirection, true)) return;

    // Allow changing Sonic's rotation when charging spindash
    alignas(16) float dir[4] = { worldDirection.x(), worldDirection.y(), worldDirection.z(), 0 };
    originalCSonicRotationAdvance(This, dir, c_funcMaxTurnRate, c_funcTurnRateMultiplier, true, c_funcMaxTurnRate);
}

HOOK(int*, __fastcall, CSonicStateSquatEnd, 0x12309A0, void* This)
{
    spinDashSoundHandle.reset();
    if (NextGenPhysics::m_isSpindash)
    {
        // Play spindash launch sfx
        static SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 80041024, 1);
    }
    return originalCSonicStateSquatEnd(This);
}

HOOK(int, __fastcall, CSonicStateSlidingBegin, 0x11D7110, void* This)
{
    if (NextGenPhysics::m_isSpindash)
    {
        // Spin animation over slide
        WRITE_MEMORY(0x11D7124, uint32_t, 0x15F84F4);
        WRITE_MEMORY(0x11D6E6A, uint32_t, 0x15F84F4);
        WRITE_MEMORY(0x11D6EDB, uint32_t, 0x15F84F4);

        // Disable sliding sfx and voice
        WRITE_MEMORY(0x11D722C, int, -1);
        WRITE_MEMORY(0x11D72DC, int, -1);
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

        NextGenPhysics::m_isSliding = true;
    }

    NextGenPhysics::m_slidingTime = NextGenPhysics::m_isSpindash ? c_spindashTime : c_slidingTime;
    return originalCSonicStateSlidingBegin(This);
}

HOOK(void, __fastcall, CSonicStateSlidingAdvance, 0x11D69A0, void* This)
{
    originalCSonicStateSlidingAdvance(This);

    bool bDown, bPressed, bReleased;
    NextGenPhysics::getActionButtonStates(bDown, bPressed, bReleased);
    NextGenPhysics::m_slidingTime -= Application::getDeltaTime();
    if (bPressed || NextGenPhysics::m_slidingTime <= 0.0f)
    {
        if (NextGenPhysics::m_isSpindash)
        {
            // Cancel spindash, this will still do sweep kick and will also allow jumping before it
            StateManager::ChangeState(StateAction::Walk, *PLAYER_CONTEXT);
            return;
        }
        else
        {
            // Cancel sliding
            slidingEndWasSliding = NextGenPhysics::m_isSliding;
            StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
            return;
        }
    }
    
    // For 2D slide/spindash, there's one frame delay before Sonic can goto max speed, lower the minSpeed
    float minSpeed = (NextGenPhysics::m_isSpindash ? c_spindashSpeed : c_slidingSpeedMin) - 5.0f;
    minSpeed = Common::IsPlayerIn2D() ? 2.0f : minSpeed;

    Eigen::Vector3f playerVelocity;
    bool result = Common::IsPlayerIn2D() ? Common::GetPlayerTargetVelocity(playerVelocity) : Common::GetPlayerVelocity(playerVelocity);
    if (!result || playerVelocity.norm() <= minSpeed)
    {
        slidingEndWasSliding = NextGenPhysics::m_isSliding;
        StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
        return;
    }
}

HOOK(bool, __stdcall, BActionHandler, 0xDFF660, CSonicContext* context, bool buttonHoldCheck)
{
    bool result = originalBActionHandler(context, buttonHoldCheck);
    if (result || Common::IsPlayerControlLocked())
    {
        NextGenPhysics::m_bHeldTimer = 0.0f;
        return result;
    }

    return NextGenPhysics::bActionHandlerImpl();
}

HOOK(int, __fastcall, CSonicStateSlidingEndBegin, 0x1230F80, void* This)
{
    // For Sonic only, do a flip if no stick input
    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        Eigen::Vector3f inputDirection;
        if (Common::GetWorldInputDirection(inputDirection) && inputDirection.isZero())
        {
            // Do brake flip animation
            NextGenPhysics::m_isBrakeFlip = true;
            WRITE_MEMORY(0x1230F88, char*, AnimationSetPatcher::BrakeFlip);
        }
        else
        {
            // Original SlidingToWalk animation
            WRITE_MEMORY(0x1230F88, uint32_t, 0x15E6A00);
        }
    }

    return originalCSonicStateSlidingEndBegin(This);
}

HOOK(int*, __fastcall, CSonicStateSlidingEndAdvance, 0x1230EE0, void* This)
{
    int* result = originalCSonicStateSlidingEndAdvance(This);
    if (NextGenPhysics::m_isBrakeFlip)
    {
        if (!slidingEndWasSliding)
        {
            // Only detect B-action during the flip, not normal SlidingEnd
            NextGenPhysics::bActionHandlerImpl();
        }

        // Enforce brake flip rotation
        alignas(16) float dir[4] = { NextGenPhysics::m_brakeFlipDir.x(), NextGenPhysics::m_brakeFlipDir.y(), NextGenPhysics::m_brakeFlipDir.z(), 0 };
        originalCSonicRotationAdvance(This, dir, c_funcMaxTurnRate, c_funcTurnRateMultiplier, true, c_funcMaxTurnRate);
    }
    return result;
}

HOOK(int*, __fastcall, CSonicStateSlidingEndEnd, 0x1230E60, void* This)
{
    NextGenPhysics::m_isBrakeFlip = false;
    return originalCSonicStateSlidingEndEnd(This);
}

uint32_t noAirDashOutOfControlReturnAddress = 0x1232445;
uint32_t noAirDashOutOfControlSkipAddress = 0x1232450;
void __declspec(naked) noAirDashOutOfControl()
{
    __asm
    {
        mov     byte ptr [ecx + 17h], 0

        // Check where we have lock-on target
        cmp     dword ptr[ebx + 0E98h], 0
        jnz     jump

        // Air dash, skip out of control
        mov     [esi + 80h], 0
        jmp     [noAirDashOutOfControlSkipAddress]

        // Out of control prep
        jump:
        push    ecx
        fstp    [esp]
        push    ebx
        jmp     [noAirDashOutOfControlReturnAddress]
    }
}

uint32_t bounceBraceletASMImplReturnAddress = 0x1254B77;
void __declspec(naked) bounceBraceletASMImpl()
{
    __asm
    {
        call    NextGenPhysics::bounceBraceletImpl
        jmp     [bounceBraceletASMImplReturnAddress]
    }
}

uint32_t lightDashHigherPriorityReturnAddress = 0xDFDDD0;
uint32_t lightDashHigherPrioritySuccessAddress = 0xDFDDED;
uint32_t fpLightSpeedDash = 0xDFB3F0;
void __declspec(naked) lightDashHigherPriority()
{
    __asm
    {
        // Check light speed dash
        call    [fpLightSpeedDash]
        test    al, al
        jz      jump
        jmp     [lightDashHigherPrioritySuccessAddress]

        // Original check B-button actions
        jump:
        push    [0x15F55A4]
        jmp     [lightDashHigherPriorityReturnAddress]
    }
}

uint32_t slidingHorizontalTargetVel2DReturnAddress = 0x11D98AC;
void __declspec(naked) slidingHorizontalTargetVel2D()
{
    __asm
    {
        // Get overrided target velocity
        mov     ecx, esi
        call    NextGenPhysics::applySlidingHorizontalTargetVel
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        jmp     [slidingHorizontalTargetVel2DReturnAddress]
    }
}

uint32_t slidingHorizontalTargetVel3DReturnAddress = 0x11D953E;
void __declspec(naked) slidingHorizontalTargetVel3D()
{
    __asm
    {
        // Get overrided target velocity
        mov     ecx, ebx
        call    NextGenPhysics::applySlidingHorizontalTargetVel
        test    al, al
        jnz     jump

        // Original target velocity fallback code
        movaps  xmm0, [esp + 10h]
        movaps  xmmword ptr[ebx + 2A0h], xmm0

        jump:
        jmp     [slidingHorizontalTargetVel3DReturnAddress]
    }
}

uint32_t startSpindashReturnAddress = 0x1230C39;
void __declspec(naked) startSpindash()
{
    __asm
    {
        // Original function
        mov     byte ptr[ebx + 5E8h], 1
        mov     [ebx + 5E9h], al

        // Give Sonic the initial nudge
        mov     ecx, ebx
        call    NextGenPhysics::applySpindashImpulse

        // Set spindash state
        mov     NextGenPhysics::m_isSpindash, 1
        jmp     [startSpindashReturnAddress]
    }
}

uint32_t CSonicStateSlidingEndReturnAddress = 0x11D702D;
void __declspec(naked) CSonicStateSlidingEnd()
{
    __asm
    {
        // Original function
        mov     eax, [edi + 534h]

        // Set spindash/sliding state
        mov     NextGenPhysics::m_isSliding, 0
        mov     NextGenPhysics::m_isSpindash, 0
        jmp     [CSonicStateSlidingEndReturnAddress]
    }
}

uint32_t loseAllRingsReturnAddress = 0xE6621E;
void __declspec(naked) loseAllRings()
{
    __asm
    {
        mov     dword ptr [edi + 5B8h], 0
        jmp     [loseAllRingsReturnAddress]
    }
}

void NextGenPhysics::applyPatches()
{
    // TODO: No trick rainbow ring but keep the animation?
    // Change character movement aniamtion speeds
    applyCharacterAnimationSpeed();

    // Always disable jump ball sfx
    WRITE_MEMORY(0x11BCC7E, int, -1);

    // Maintain down speed when homing attack finished (for 06 physics)
    INSTALL_HOOK(CSonicStateHomingAttackBegin);
    INSTALL_HOOK(CSonicStateHomingAttackEnd);

    // Always disable stomp voice and sfx for Sonic
    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        WRITE_MEMORY(0x1254E04, int, -1);
        WRITE_MEMORY(0x1254F23, int, -1);
    }

    // Do SpinFall animation when Sonic starts to fall
    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        // Transition to Fall immediately without checking down speed for CPlayerSpeedStateSpecialJump
        WRITE_JUMP(0x11DE320, (void*)0x11DE344);

        // Don't transition to FallLarge when speed < -15.0f
        WRITE_MEMORY(0x1118DE5, uint8_t, 0xEB);

        // Change animation at CSonicStateFall base on current animation
        INSTALL_HOOK(CSonicStateFallBegin);
        INSTALL_HOOK(CSonicStateFallAdvance);
    }

    // 06 physics general code
    if (Configuration::m_physics)
    {
        // Increase turning rate
        INSTALL_HOOK(CSonicRotationAdvance);

        // No out of control for air dash
        WRITE_JUMP(0x123243C, noAirDashOutOfControl);

        // Disable jumpball collision
        INSTALL_HOOK(CSonicStateJumpBallBegin);
        WRITE_MEMORY(0x1118FFA, uint8_t, 0xC2, 0xC5); // Fall state

        // Set rings to 0 when getting damaged
        WRITE_JUMP(0xE66218, loseAllRings);
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
        //WRITE_MEMORY(0x11BD057, uint32_t, 32);  // DivingDive start
        //WRITE_MEMORY(0x124AF01, uint32_t, 32);  // DivingDive end

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

    if (!Configuration::m_characterMoveset) return;

    //-------------------------------------------------------
    // B-Action State handling
    //-------------------------------------------------------
    // Return 0 for Squat and Sliding, handle them ourselves
    WRITE_MEMORY(0xDFF8D5, uint8_t, 0xEB, 0x05);
    WRITE_MEMORY(0xDFF856, uint8_t, 0xE9, 0x81, 0x00, 0x00, 0x00);
    INSTALL_HOOK(BActionHandler);
    INSTALL_HOOK(CSonicStateSlidingEndBegin);
    INSTALL_HOOK(CSonicStateSlidingEndAdvance);
    INSTALL_HOOK(CSonicStateSlidingEndEnd);

    //-------------------------------------------------------
    // Bounce Bracelet
    //-------------------------------------------------------
    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        INSTALL_HOOK(CSonicStateGrounded);
        INSTALL_HOOK(CSonicStateStompingBegin);
        WRITE_JUMP(0x1254A36, bounceBraceletASMImpl);
        WRITE_JUMP(0x12549C9, bounceBraceletASMImpl);

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
    }

    //-------------------------------------------------------
    // Sweep Kick
    //-------------------------------------------------------
    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        // Play squat kick sfx
        INSTALL_HOOK(CSonicStateSquatKickBegin);
        INSTALL_HOOK(CSonicStateSquatKickAdvance);
        INSTALL_HOOK(CSonicStateSquatKickEnd);
        if (Configuration::m_model == Configuration::ModelType::Sonic
         && Configuration::m_language == Configuration::LanguageType::English)
        {
            WRITE_MEMORY(0x1252740, uint32_t, 3002020);
        }
        else
        {
            // Prevent stopping other voice with low priority
            WRITE_MEMORY(0x1252740, int, -1);
            WRITE_MEMORY(0x1252732, uint8_t, 0);
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
    if (Configuration::m_model == Configuration::ModelType::Sonic
     || Configuration::m_model == Configuration::ModelType::SonicElise)
    {
        // Change slide to hit enemy as if you're squat kicking (TypeSonicSquatKick)
        WRITE_MEMORY(0x11D72F3, uint32_t, SonicCollision::TypeSonicSquatKick);
        WRITE_MEMORY(0x11D7090, uint32_t, SonicCollision::TypeSonicSquatKick);

        // Disable all Sliding transition out, handle them outselves
        WRITE_MEMORY(0x11D6B7D, uint8_t, 0xEB);
        WRITE_MEMORY(0x11D6CA2, uint8_t, 0xEB);
        WRITE_MEMORY(0x11D6F82, uint8_t, 0x90, 0xE9);

        // Change sliding animation if we are spindashing, handle transition out
        INSTALL_HOOK(CSonicStateSlidingBegin);
        INSTALL_HOOK(CSonicStateSlidingAdvance);
        WRITE_JUMP(0x11D7027, CSonicStateSlidingEnd);

        // Set constant sliding speed
        WRITE_JUMP(0x11D989B, slidingHorizontalTargetVel2D);
        WRITE_JUMP(0x11D98A7, slidingHorizontalTargetVel2D);
        WRITE_JUMP(0x11D9532, slidingHorizontalTargetVel3D);
    }

    //-------------------------------------------------------
    // Spindashing
    //-------------------------------------------------------
    if (Configuration::m_model == Configuration::ModelType::Sonic)
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
        INSTALL_HOOK(CSonicStateSquatBegin);
        INSTALL_HOOK(CSonicStateSquatAdvance);
        INSTALL_HOOK(CSonicStateSquatEnd);
    }
}

void NextGenPhysics::applyCharacterAnimationSpeed()
{
    // Note: All animations have 81 frames [0-80]
    // playbackSpeed: how fast animation plays
    // speedFactor: how much distance to play one loop, -1.0 to use constant playbackSpeed

    static float walk_playbackSpeed = 1.0f;
    static float walk_speedFactor = 1.35f;
    static float walkFast_playbackSpeed = 1.0f;
    static float walkFast_speedFactor = 2.0f;

    static float jog_playbackSpeed = 1.0f;
    static float jog_speedFactor = 3.3f;

    static float run_playbackSpeed = 1.0f;
    static float run_speedFactor = 9.0f;

    static float dash_playbackSpeed = 1.0f;
    static float dash_speedFactor = 12.0f;

    static float jet_playbackSpeed = 1.0f;
    static float jet_speedFactor = 13.0f;
    static float jetWall_playbackSpeed = 1.0f;
    static float jetWall_speedFactor = 20.0f;

    static float boost_playbackSpeed = 1.0f;
    static float boost_speedFactor = 13.0f;
    static float boostWall_playbackSpeed = 1.0f;
    static float boostWall_speedFactor = 20.0f;

    // Modifying movement animation speed
    switch (Configuration::m_model)
    {
        case Configuration::ModelType::Sonic:
        {
            if (Configuration::m_physics)
            {
                jog_speedFactor = 2.7f;
                run_speedFactor = 3.2f;
                dash_speedFactor = 5.0f;
                jet_playbackSpeed = 6.0f;
                jet_speedFactor = -1.0f;
                jetWall_playbackSpeed = 6.0f;
                jetWall_speedFactor = -1.0f;
                boost_playbackSpeed = 7.0f;
                boost_speedFactor = -1.0f;
                boostWall_playbackSpeed = 7.0f;
                boostWall_speedFactor = -1.0f;
            }
            else
            {
                run_speedFactor = 6.0f;
                dash_speedFactor = 9.0f;
            }
            break;
        }
        case Configuration::ModelType::SonicElise:
        {
            if (Configuration::m_physics)
            {
                run_speedFactor = 5.0f;
                dash_speedFactor = 7.0f;
            }
            else
            {
                run_speedFactor = 6.0f;
                dash_speedFactor = 9.0f;
            }
            break;
        }
    }

    WRITE_MEMORY(0x12724AA, float*, &walk_playbackSpeed);
    WRITE_MEMORY(0x12724D7, float*, &walk_speedFactor);

    WRITE_MEMORY(0x127251B, float*, &walkFast_playbackSpeed);
    WRITE_MEMORY(0x1272546, float*, &walkFast_speedFactor);

    WRITE_MEMORY(0x127258A, float*, &jog_playbackSpeed);
    WRITE_MEMORY(0x12725F9, float*, &jog_playbackSpeed);
    WRITE_MEMORY(0x1272668, float*, &jog_playbackSpeed);
    WRITE_MEMORY(0x12725B5, float*, &jog_speedFactor);
    WRITE_MEMORY(0x1272624, float*, &jog_speedFactor);
    WRITE_MEMORY(0x1272693, float*, &jog_speedFactor);

    WRITE_MEMORY(0x12726D7, float*, &run_playbackSpeed);
    WRITE_MEMORY(0x1272746, float*, &run_playbackSpeed);
    WRITE_MEMORY(0x12727B5, float*, &run_playbackSpeed);
    WRITE_MEMORY(0x1272702, float*, &run_speedFactor);
    WRITE_MEMORY(0x1272771, float*, &run_speedFactor);
    WRITE_MEMORY(0x12727E0, float*, &run_speedFactor);

    WRITE_MEMORY(0x1272824, float*, &dash_playbackSpeed);
    WRITE_MEMORY(0x1272893, float*, &dash_playbackSpeed);
    WRITE_MEMORY(0x1272902, float*, &dash_playbackSpeed);
    WRITE_MEMORY(0x127284F, float*, &dash_speedFactor);
    WRITE_MEMORY(0x12728BE, float*, &dash_speedFactor);
    WRITE_MEMORY(0x127292D, float*, &dash_speedFactor);

    WRITE_MEMORY(0x1272971, float*, &jet_playbackSpeed);
    WRITE_MEMORY(0x12729E0, float*, &jet_playbackSpeed);
    WRITE_MEMORY(0x1272A4F, float*, &jet_playbackSpeed);
    WRITE_MEMORY(0x1272ABE, float*, &jetWall_playbackSpeed);
    WRITE_MEMORY(0x1272B2D, float*, &jetWall_playbackSpeed);
    WRITE_MEMORY(0x127299C, float*, &jet_speedFactor);
    WRITE_MEMORY(0x1272A0B, float*, &jet_speedFactor);
    WRITE_MEMORY(0x1272A7A, float*, &jet_speedFactor);
    WRITE_MEMORY(0x1272AE9, float*, &jetWall_speedFactor);
    WRITE_MEMORY(0x1272B58, float*, &jetWall_speedFactor);

    WRITE_MEMORY(0x1272B9C, float*, &boost_playbackSpeed);
    WRITE_MEMORY(0x1272C0B, float*, &boost_playbackSpeed);
    WRITE_MEMORY(0x1272C7A, float*, &boost_playbackSpeed);
    WRITE_MEMORY(0x1272CE9, float*, &boostWall_playbackSpeed);
    WRITE_MEMORY(0x1272D58, float*, &boostWall_playbackSpeed);
    WRITE_MEMORY(0x1272BC7, float*, &boost_speedFactor);
    WRITE_MEMORY(0x1272C36, float*, &boost_speedFactor);
    WRITE_MEMORY(0x1272CA5, float*, &boost_speedFactor);
    WRITE_MEMORY(0x1272D14, float*, &boostWall_speedFactor);
    WRITE_MEMORY(0x1272D83, float*, &boostWall_speedFactor);
}

void NextGenPhysics::bounceBraceletImpl()
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
    velocity[1] = NextGenPhysics::m_bounced ? c_bouncePowerBig : c_bouncePower;
    if (flags->OnWater)
    {
        velocity[1] -= 5.0f;
    }
    velocity[2] = 0.0f;
    NextGenPhysics::m_bounced = true;
    StateManager::ChangeState(StateAction::Jump, *pModernSonicContext);

    // Set out of control
    FUNCTION_PTR(int, __stdcall, SetOutOfControl, 0xE5AC00, CSonicContext * context, float duration);
    SetOutOfControl(*pModernSonicContext, 0.1f);
}

bool __fastcall NextGenPhysics::applySpindashImpulse(void* context)
{
    // This function is necessary for 2D otherwise Sonic will stop immediately
    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (!Common::GetPlayerTransform(playerPosition, playerRotation)) return false;

    Eigen::Vector3f playerVelocity;
    if (!Common::GetPlayerVelocity(playerVelocity)) return false;

    MsgAddImpulse message;
    message.m_position = playerPosition;
    message.m_impulse = playerRotation * Eigen::Vector3f::UnitZ();
    message.m_impulseType = ImpulseType::None;
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
        float hSpeed = playerVelocity.norm();
        hSpeed = max(hSpeed, c_slidingSpeedMin);
        hSpeed = min(hSpeed, c_slidingSpeedMax);
        message.m_impulse *= hSpeed;
    }

    Common::ApplyPlayerAddImpulse(message);

    return true;
}

bool __fastcall NextGenPhysics::applySlidingHorizontalTargetVel(void* context)
{
    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (!Common::GetPlayerTransform(playerPosition, playerRotation)) return false;
    Eigen::Vector3f playerDir = playerRotation * Eigen::Vector3f::UnitZ();

    Eigen::Vector3f playerVelocity;
    if (Common::IsPlayerIn2D())
    {
        Common::GetPlayerTargetVelocity(playerVelocity);
    }
    else
    {
        Common::GetPlayerVelocity(playerVelocity);
    }

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

    NextGenPhysics::m_brakeFlipDir = playerDir;
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

        float hSpeed = playerVelocity.norm();
        hSpeed = max(hSpeed, c_slidingSpeedMin);
        hSpeed = min(hSpeed, c_slidingSpeedMax);
        playerDir *= hSpeed;
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

void NextGenPhysics::getActionButtonStates(bool& bDown, bool& bPressed, bool& bReleased)
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

bool NextGenPhysics::bActionHandlerImpl()
{
    Eigen::Vector3f playerVelocity;
    if (!Common::GetPlayerVelocity(playerVelocity))
    {
        return false;
    }
    bool moving = playerVelocity.norm() > 0.2f;

    bool bDown, bPressed, bReleased;
    NextGenPhysics::getActionButtonStates(bDown, bPressed, bReleased);
    CSonicStateFlags* flags = Common::GetSonicStateFlags();
    if (bDown)
    {
        // Standing still and held B for a while (Spin Dash)
        if (!moving && NextGenPhysics::m_bHeldTimer > c_squatKickPressMaxTime)
        {
            if (Configuration::m_model == Configuration::ModelType::Sonic)
            {
                StateManager::ChangeState(StateAction::Squat, *PLAYER_CONTEXT);
                NextGenPhysics::m_bHeldTimer = 0.0f;
                return true;
            }
        }

        // Remember how long we held B
        NextGenPhysics::m_bHeldTimer += Application::getDeltaTime();
    }
    else
    {
        if (bReleased && !flags->OnWater)
        {
            if (NextGenPhysics::m_bHeldTimer <= c_squatKickPressMaxTime)
            {
                if (Configuration::m_model == Configuration::ModelType::Sonic)
                {
                    // Release B without holding it for too long (Squat Kick)
                    StateManager::ChangeState(StateAction::SquatKick, *PLAYER_CONTEXT);
                    NextGenPhysics::m_bHeldTimer = 0.0f;
                    return true;
                }
            }
            else if (moving && NextGenPhysics::m_bHeldTimer > c_squatKickPressMaxTime && !flags->KeepRunning)
            {
                // Sonic is moving and released B (Anti-Gravity)
                StateManager::ChangeState(StateAction::Sliding, *PLAYER_CONTEXT);
                NextGenPhysics::m_bHeldTimer = 0.0f;
                return true;
            }
        }

        NextGenPhysics::m_bHeldTimer = 0.0f;
    }

    return false;
}
