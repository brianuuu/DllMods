#include "NextGenPhysics.h"
#include "Configuration.h"
#include "StateManager.h"
#include "Application.h"

float const c_funcMaxTurnRate = 400.0f;
float const c_funcTurnRateMultiplier = PI_F * 10.0f;

bool NextGenPhysics::m_isStomping = false;
bool NextGenPhysics::m_bounced = false;
float const c_bouncePower = 17.0f;
float const c_bouncePowerBig = 23.0f;

bool NextGenPhysics::m_isSquatKick = false;
float const c_squatKickPressMaxTime = 0.3f;

bool NextGenPhysics::m_isSpindash = false;
bool NextGenPhysics::m_isSliding2D = false;
float NextGenPhysics::m_slidingTime = 0.0f;
float const c_slidingTime = 3.0f;
float const c_slidingSpeedMin = 10.0f;
float const c_slidingSpeedMax = 16.0f;
float const c_spindashTime = 3.0f;
float const c_spindashSpeed = 30.0f;

HOOK(void, __cdecl, SetStickMagnitude, 0x9C69D0, int16_t* argX, int16_t* argY, int16_t a3, int16_t Deadzone)
{
    const double x = min(1.0, max(-1.0, *argX / 32767.0));
    const double y = min(1.0, max(-1.0, *argY / 32767.0));
    const double magnitude = sqrt(x * x + y * y);
    const double deadzone = Deadzone / 32767.0;
    const double newMagnitude = max(0.0, min(1.0, (magnitude - deadzone) / (1 - deadzone)));

    *argX = (int16_t)(x / magnitude * newMagnitude * 32767.0);
    *argY = (int16_t)(y / magnitude * newMagnitude * 32767.0);
}

HOOK(void, __stdcall, CSonicRotationAdvance, 0xE310A0, void* a1, float* targetDir, float turnRate1, float turnRateMultiplier, bool noLockDirection, float turnRate2)
{
    CSonicContext* context = (CSonicContext*)(((uint32_t*)a1)[2]);
    if (noLockDirection)
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

HOOK(int, __fastcall, CSonicStateStompingBegin, 0x1254CA0, void* This)
{
    NextGenPhysics::m_isStomping = true;
    return originalCSonicStateStompingBegin(This);
}

HOOK(int, __fastcall, CSonicStateHomingAttackBegin, 0x1232040, void* This)
{
    NextGenPhysics::m_bounced = false;
    return originalCSonicStateHomingAttackBegin(This);
}

HOOK(int*, __fastcall, CSonicStateSquatKickBegin, 0x12526D0, void* This)
{
    // Don't allow direction change for squat kick
    WRITE_MEMORY(0x11D944A, uint8_t, 0);

    static SharedPtrTypeless soundHandle;
    Common::SonicContextPlaySound(soundHandle, 80041021, 1);
    NextGenPhysics::m_isSquatKick = true;
    return originalCSonicStateSquatKickBegin(This);
}

// TODO: When transition out from squat kick/sliding, if no stick input, stop and flip Sonic?
HOOK(void, __fastcall, CSonicStateSquatKickAdvance, 0x1252810, void* This)
{
    originalCSonicStateSquatKickAdvance(This);

    // About to transition out and on ground
    CSonicStateFlags* flags = Common::GetSonicStateFlags();
    if (!flags->KeepRunning
        && !StateManager::isCurrentAction(StateAction::SquatKick)
        && !StateManager::isCurrentAction(StateAction::Fall))
    {
        bool bDown, bPressed, bReleased;
        NextGenPhysics::getActionButtonStates(bDown, bPressed, bReleased);
        Sonic::EKeyState actionButton = Configuration::m_xButtonAction ? Sonic::EKeyState::eKeyState_X : Sonic::EKeyState::eKeyState_B;
        if (bDown)
        {
            // Immediately change to spindash if button is held
            // TODO: do a flip first
            StateManager::ChangeState(StateAction::Squat, *PLAYER_CONTEXT);
            return;
        }

        Eigen::Vector3f inputDirection;
        if (Common::GetWorldInputDirection(inputDirection) && inputDirection.isZero())
        {
            // Stops Sonic completely if not stick input
            // TODO: do a flip first
            Common::SetPlayerVelocity(Eigen::Vector3f::Zero());
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
            StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
            return;
        }
    }
    
    // For 2D slide/spindash, there's one frame delay before Sonic can goto max speed, lower the minSpeed
    float minSpeed = (NextGenPhysics::m_isSpindash ? c_spindashSpeed : c_slidingSpeedMin) - 5.0f;
    minSpeed = NextGenPhysics::m_isSliding2D ? 2.0f : minSpeed;

    Eigen::Vector3f playerVelocity;
    bool result = NextGenPhysics::m_isSliding2D ? Common::GetPlayerTargetVelocity(playerVelocity) : Common::GetPlayerVelocity(playerVelocity);
    if (!result || playerVelocity.norm() <= minSpeed)
    {
        StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
        return;
    }
}

HOOK(bool, __stdcall, BActionHandler, 0xDFF660, CSonicContext* context, bool buttonHoldCheck)
{
    static float bHeldTimer(0.0f);

    bool* unknownFlags = *(bool**)((uint32_t)context + 0x11C);
    bool result = originalBActionHandler(context, buttonHoldCheck);
    if (result || unknownFlags[0x98] || unknownFlags[0x99])
    {
        bHeldTimer = 0.0f;
        return result;
    }

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
        if (!moving && bHeldTimer > c_squatKickPressMaxTime)
        {
            if (Configuration::m_model == Configuration::ModelType::Sonic)
            {
                StateManager::ChangeState(StateAction::Squat, *PLAYER_CONTEXT);
                bHeldTimer = 0.0f;
                return true;
            }
        }

        // Remember how long we held B
        bHeldTimer += Application::getDeltaTime();
    }
    else
    {
        if (bReleased && !flags->OnWater)
        {
            if (bHeldTimer <= c_squatKickPressMaxTime)
            {
                if (Configuration::m_model == Configuration::ModelType::Sonic)
                {
                    // Release B without holding it for too long (Squat Kick)
                    StateManager::ChangeState(StateAction::SquatKick, *PLAYER_CONTEXT);
                    bHeldTimer = 0.0f;
                    return true;
                }
            }
            else if (moving && bHeldTimer > c_squatKickPressMaxTime && !flags->KeepRunning)
            {
                // Sonic is moving and released B (Anti-Gravity)
                StateManager::ChangeState(StateAction::Sliding, *PLAYER_CONTEXT);
                bHeldTimer = 0.0f;
                return true;
            }
        }

        bHeldTimer = 0.0f;
    }

    return false;
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
uint32_t fpLightSpeedDash = 0xDFFD10;
void __declspec(naked) lightDashHigherPriority()
{
    __asm
    {
        // Check light speed dash
        mov     eax, esi
        mov     ebx, [esp]
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
        mov     NextGenPhysics::m_isSliding2D, 1
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
        mov     NextGenPhysics::m_isSliding2D, 0
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

        // Set spindash state
        mov     NextGenPhysics::m_isSpindash, 0
        jmp     [CSonicStateSlidingEndReturnAddress]
    }
}

void NextGenPhysics::applyPatches()
{
    // Always disable stomp voice and sfx for Sonic
    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        WRITE_MEMORY(0x1254E04, int, -1);
        WRITE_MEMORY(0x1254F23, int, -1);
    }

    // Never transition to Fall after jumpboard, must use long jumpboard animation
    WRITE_MEMORY(0x11DE31E, uint8_t, 0xEB);

    if (!Configuration::m_physics) return;

    // Precise stick input, by Skyth (06 still has angle clamp, don't use)
    // INSTALL_HOOK(SetStickMagnitude);

    // Increase turning rate
    INSTALL_HOOK(CSonicRotationAdvance);

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
    }

    // No out of control for air dash
    WRITE_JUMP(0x123243C, noAirDashOutOfControl);

    // Implement bounce bracelet
    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        INSTALL_HOOK(CSonicStateGrounded);
        INSTALL_HOOK(CSonicStateStompingBegin);
        INSTALL_HOOK(CSonicStateHomingAttackBegin);
        WRITE_JUMP(0x1254A36, bounceBraceletASMImpl);
        WRITE_JUMP(0x12549C9, bounceBraceletASMImpl);

        // Disable jump ball sfx
        WRITE_MEMORY(0x11BCC7E, int, -1);

        // Replace stomp land sfx with bounce
        WRITE_MEMORY(0x12548FC, uint32_t, 80041022);
        WRITE_MEMORY(0x12549D8, uint32_t, 80041022);

        // Allow spin attack OnWater
        WRITE_MEMORY(0x12352B8, uint8_t, 0xEB);

        // Skip B action check on Stomping
        WRITE_NOP(0x12549CF, 0x8);
        WRITE_NOP(0x12549DC, 0x2);
        WRITE_MEMORY(0x12549DE, uint8_t, 0xEB);

        // Change jumpball to hit enemy as if you're boosting
        WRITE_MEMORY(0x11BCC43, uint32_t, 0x1E61B90); // jumpball start
        WRITE_MEMORY(0x11BCBB2, uint32_t, 0x1E61B90); // jumpball end
    }

    //-------------------------------------------------------
    // B-Action State handling
    //-------------------------------------------------------
    // Return 0 for Squat and Sliding, handle them ourselves
    WRITE_MEMORY(0xDFF8D5, uint8_t, 0xEB, 0x05);
    WRITE_MEMORY(0xDFF856, uint8_t, 0xE9, 0x81, 0x00, 0x00, 0x00);
    INSTALL_HOOK(BActionHandler);

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
        // Change slide to hit enemy as if you're boosting
        WRITE_MEMORY(0x11D72F3, uint32_t, 0x1E61B90);
        WRITE_MEMORY(0x11D7090, uint32_t, 0x1E61B90);

        // Disable all Sliding transition out, handle them outselves
        WRITE_MEMORY(0x11D6B7D, uint8_t, 0xEB);
        WRITE_MEMORY(0x11D6CA2, uint8_t, 0xEB);
        WRITE_MEMORY(0x11D6F82, uint8_t, 0x90, 0xE9);

        // Change sliding animation if we are spindashing, handle transition out
        INSTALL_HOOK(CSonicStateSlidingBegin);
        INSTALL_HOOK(CSonicStateSlidingAdvance);
        WRITE_JUMP(0x11D7027, CSonicStateSlidingEnd);

        // Set constant sliding speed
        // TODO: 2D spindash not working
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
        WRITE_MEMORY(0x1230A84, uint32_t, 0x15F84F4); // slide begin animation
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
    StateManager::ChangeState(StateAction::Jump, *pModernSonicContext);
    NextGenPhysics::m_bounced = true;

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

    if (m_isSquatKick)
    {
        // Keep velocity with squat kick
        message.m_impulse = playerVelocity;
    }
    else if (m_isSpindash)
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
    if (!Common::GetPlayerVelocity(playerVelocity)) return false;

    if (m_isSquatKick)
    {
        // Keep velocity with squat kick
        playerDir = playerVelocity;
    }
    else if (m_isSpindash)
    {
        playerDir *= c_spindashSpeed;
    }
    else
    {
        float hSpeed = playerVelocity.norm();
        hSpeed = max(hSpeed, c_slidingSpeedMin);
        hSpeed = min(hSpeed, c_slidingSpeedMax);
        playerDir *= hSpeed;
    }

    // For 2D we have to override the actual velocity (+0x290)
    // For 3D we have to override target velocity (+0x2A0)
    float* horizontalVel = (float*)((uint32_t)context + (NextGenPhysics::m_isSliding2D ? 0x290 : 0x2A0));
    horizontalVel[0] = playerDir.x();
    if (NextGenPhysics::m_isSliding2D)
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
