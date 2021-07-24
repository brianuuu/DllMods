#include "NextGenPhysics.h"
#include "Configuration.h"
#include "StateManager.h"
#include "Application.h"

bool NextGenPhysics::m_isStomping = false;
bool NextGenPhysics::m_bounced = false;
float const c_bouncePower = 17.0f;
float const c_bouncePowerBig = 23.0f;

bool NextGenPhysics::m_isSquatKick = false;
float const c_squatKickPressMaxTime = 0.3f;

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

HOOK(void, __fastcall, CSonicStateSquatKickAdvance, 0x1252810, void* This)
{
    if (!NextGenPhysics::m_isSquatKick)
    {
        static SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 80041021, 1);
        NextGenPhysics::m_isSquatKick = true;
    }
    originalCSonicStateSquatKickAdvance(This);
}

HOOK(int*, __fastcall, CSonicStateSquatKickEnd, 0x12527B0, void* This)
{
    NextGenPhysics::m_isSquatKick = false;
    return originalCSonicStateSquatKickEnd(This);
}

HOOK(bool, __stdcall, BActionHandler, 0xDFF660, CSonicContext* context, bool buttonHoldCheck)
{
    static float bHeldTimer(0.0f);

    bool result = originalBActionHandler(context, buttonHoldCheck);
    if (result)
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

void NextGenPhysics::applyPatches()
{
    if (!Configuration::m_physics) return;

    // Precise stick input, by Skyth (06 still has angle clamp, don't use)
    // INSTALL_HOOK(SetStickMagnitude);

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

        // Disable stomp voice and sfx
        WRITE_MEMORY(0x1254E04, int, -1);
        WRITE_MEMORY(0x1254F23, int, -1);

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
    }

    // Implement sweep kick, anti-gravity and spindash
    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        // Return 0 for Squat and Sliding, handle them ourselves
        WRITE_MEMORY(0xDFF8D5, uint8_t, 0xEB, 0x05);
        WRITE_MEMORY(0xDFF856, uint8_t, 0xE9, 0x81, 0x00, 0x00, 0x00);
        INSTALL_HOOK(BActionHandler);

        // Play squat kick sfx
        INSTALL_HOOK(CSonicStateSquatKickAdvance);
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

        // Change slide to hit enemy as if you're boosting
        WRITE_MEMORY(0x11D72F3, uint32_t, 0x1E61B90);
        WRITE_MEMORY(0x11D7090, uint32_t, 0x1E61B90);

        // Spin animation for squat
        WRITE_MEMORY(0x1230A84, uint32_t, 0x15F84F4);
        WRITE_MEMORY(0x1230A9F, uint32_t, 0x15F84F4);
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
