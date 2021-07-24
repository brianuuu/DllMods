#include "NextGenPhysics.h"
#include "Configuration.h"
#include "StateManager.h"
#include "Application.h"

// Used by sub_E310A0
float const c_funcMaxTurnRate = 400.0f;
float const c_funcTurnRateMultiplier = PI_F * 10.0f;

bool NextGenPhysics::m_isStomping = false;
bool NextGenPhysics::m_bounced = false;
float const c_bouncePower = 17.0f;
float const c_bouncePowerBig = 23.0f;

bool NextGenPhysics::m_isSquatKick = false;
float const c_squatKickPressMaxTime = 0.3f;

bool NextGenPhysics::m_isSpindash = false;
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
    static float const zeroTurnRate = 0.0f;
    WRITE_MEMORY(0x11D9441, float*, &zeroTurnRate);

    static SharedPtrTypeless soundHandle;
    Common::SonicContextPlaySound(soundHandle, 80041021, 1);
    NextGenPhysics::m_isSquatKick = true;
    return originalCSonicStateSquatKickBegin(This);
}

HOOK(int*, __fastcall, CSonicStateSquatKickEnd, 0x12527B0, void* This)
{
    // Unlock direction change for sliding/spindash
    WRITE_MEMORY(0x11D944A, uint8_t, 1);
    WRITE_MEMORY(0x11D9441, float*, &c_funcMaxTurnRate);

    NextGenPhysics::m_isSquatKick = false;
    return originalCSonicStateSquatKickEnd(This);
}

static SharedPtrTypeless spinDashSoundHandle;
HOOK(int*, __fastcall, CSonicStateSquatBegin, 0x1230A30, void* This)
{
    Common::SonicContextPlaySound(spinDashSoundHandle, 2002042, 1);
    return originalCSonicStateSquatBegin(This);
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

    return originalCSonicStateSlidingBegin(This);
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

    Sonic::SPadState* padState = Sonic::CInputState::GetPadState();
    Sonic::EKeyState actionButton = Configuration::m_xButtonAction ? Sonic::EKeyState::eKeyState_X : Sonic::EKeyState::eKeyState_B;
    bool bDown = padState->IsDown(actionButton);
    bool bPressed = padState->IsTapped(actionButton);
    bool bReleased = padState->IsReleased(actionButton);

    CSonicStateFlags* flags = Common::GetSonicStateFlags();
    if (bDown)
    {
        // Standing still and held B for a while (Spin Dash)
        if (!moving && bHeldTimer > c_squatKickPressMaxTime)
        {
            StateManager::ChangeState(StateAction::Squat, *PLAYER_CONTEXT);
            bHeldTimer = 0.0f;
            return true;
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
                // Release B without holding it for too long (Squat Kick)
                StateManager::ChangeState(StateAction::SquatKick, *PLAYER_CONTEXT);
                bHeldTimer = 0.0f;
                return true;
            }
            else if (moving && bHeldTimer > c_squatKickPressMaxTime)
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

uint32_t slidingHorizontalTargetVel3DReturnAddress = 0x11D953E;
void __declspec(naked) slidingHorizontalTargetVel3D()
{
    __asm
    {
        // Get overrided target velocity
        mov     ecx, ebx
        call    NextGenPhysics::applySlidingHorizontalTargetVel3D
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

    if (!Configuration::m_physics) return;

    // Precise stick input, by Skyth (06 still has angle clamp, don't use)
    // INSTALL_HOOK(SetStickMagnitude);

    // Increase turning rate for spindash/sliding
    WRITE_MEMORY(0x11D9441, float*, &c_funcMaxTurnRate);
    WRITE_MEMORY(0x11D944D, float*, &c_funcTurnRateMultiplier);

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

        // Change stomp and jumpball to hit enemy as if you're boosting
        WRITE_MEMORY(0x11BCC43, uint32_t, 0x1E61B90); // jumpball start
        WRITE_MEMORY(0x11BCBB2, uint32_t, 0x1E61B90); // jumpball end
        WRITE_MEMORY(0x1254D62, uint32_t, 0x1E61B90); // stomping start
        WRITE_MEMORY(0x1254BC5, uint32_t, 0x1E61B90); // stomping end
    }

    // Implement sweep kick, anti-gravity and spindash
    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        //-------------------------------------------------------
        // State handling
        //-------------------------------------------------------
        // Return 0 for Squat and Sliding, handle them ourselves
        WRITE_MEMORY(0xDFF8D5, uint8_t, 0xEB, 0x05);
        WRITE_MEMORY(0xDFF856, uint8_t, 0xE9, 0x81, 0x00, 0x00, 0x00);
        INSTALL_HOOK(BActionHandler);

        //-------------------------------------------------------
        // Sweep Kick
        //-------------------------------------------------------
        // Play squat kick sfx
        INSTALL_HOOK(CSonicStateSquatKickBegin);
        INSTALL_HOOK(CSonicStateSquatKickEnd);
        if (Configuration::m_model == Configuration::ModelType::Sonic
         && Configuration::m_language == Configuration::LanguageType::English)
        {
            WRITE_MEMORY(0x1252740, uint32_t, 3002020);
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

        //-------------------------------------------------------
        // Spindash & Anti-Gravity
        //-------------------------------------------------------
        // Change slide to hit enemy as if you're boosting
        WRITE_MEMORY(0x11D72F3, uint32_t, 0x1E61B90);
        WRITE_MEMORY(0x11D7090, uint32_t, 0x1E61B90);

        // Spin animation for Squat
        WRITE_MEMORY(0x1230A84, uint32_t, 0x15F84F4); // slide begin animation
        WRITE_MEMORY(0x1230A9F, uint32_t, 0x15F84F4); // slide begin animation
        WRITE_MEMORY(0x1230D74, uint32_t, 0x15F84F4); // slide hold animation

        // Use spindash when release button
        WRITE_JUMP(0x1230BDB, startSpindash);
        WRITE_MEMORY(0x1230C3A, uint32_t, 0x15F5108); // change state to sliding

         // Don't allow stick move start sliding from squat
        WRITE_MEMORY(0x1230D62, uint8_t, 0xEB);
        WRITE_MEMORY(0x1230DA9, uint8_t, 0xE9, 0xA8, 0x00, 0x00, 0x00, 0x90);

        // Change SlideToSquat to SlidingEnd
        WRITE_NOP(0x11D6CA4, 0x9);
        WRITE_NOP(0x11D6CB2, 0x43);
        WRITE_MEMORY(0x11D6CF6, uint32_t, 0x15F557C); // slide end state
        WRITE_NOP(0x11D6FA4, 0x2C);
        WRITE_MEMORY(0x11D6FD1, uint32_t, 0x15F557C); // slide end state

        // Play spindash sfx
        INSTALL_HOOK(CSonicStateSquatBegin);
        INSTALL_HOOK(CSonicStateSquatEnd);

        // Change sliding animation if we are spindashing, handle transition out
        INSTALL_HOOK(CSonicStateSlidingBegin);
        WRITE_JUMP(0x11D7027, CSonicStateSlidingEnd);

        // Set constant sliding speed
        // TODO: 2D spindash not working
        WRITE_JUMP(0x11D9532, slidingHorizontalTargetVel3D);

        // TODO stop sliding/spindash after timer, press B to cancel out
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

bool __fastcall NextGenPhysics::applySlidingHorizontalTargetVel3D(void* context)
{
    // Lock velocity to Sonic's direction, which already has turning radius increased
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

    float* targetHorizontalVel = (float*)((uint32_t)context + 0x2A0);
    targetHorizontalVel[0] = playerDir.x();
    targetHorizontalVel[1] = playerDir.y();
    targetHorizontalVel[2] = playerDir.z();

    return true;
}
