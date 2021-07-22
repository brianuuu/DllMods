#include "NextGenPhysics.h"
#include "Configuration.h"
#include "StateManager.h"

bool NextGenPhysics::m_isStomping = false;
bool NextGenPhysics::m_bounced = false;
float const c_bouncePower = 17.0f;
float const c_bouncePowerBig = 23.0f;

bool NextGenPhysics::m_isSquatKick = false;

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

void NextGenPhysics::applyPatches()
{
    if (!Configuration::m_physics) return;

    // Precise stick input, by Skyth (06 still has angle clamp, don't use)
    // INSTALL_HOOK(SetStickMagnitude);

    // No out of control for air dash
    WRITE_JUMP(0x123243C, noAirDashOutOfControl);

    // Implement bounce bracelet
    if (Configuration::m_model == ModelType::Sonic)
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
    }

    // Implement sweep kick and anti-gravity
    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        // Change running Sliding to SquatKick
        WRITE_MEMORY(0xDFF857, uint32_t, 0x15F5608);

        // Play sfx
        INSTALL_HOOK(CSonicStateSquatKickAdvance);
        INSTALL_HOOK(CSonicStateSquatKickEnd);

        static double const c_sweepKickActivateTime = 0.0;
        WRITE_MEMORY(0x125299E, double*, &c_sweepKickActivateTime);
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
