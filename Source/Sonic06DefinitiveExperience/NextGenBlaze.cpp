#include "NextGenBlaze.h"
#include "Configuration.h"
#include "Application.h"
#include "AnimationSetPatcher.h"

//---------------------------------------------------
// Animation
//---------------------------------------------------
void NextGenBlaze::setAnimationSpeed_Blaze(NextGenAnimation& data)
{
    if (Configuration::m_physics)
    {
        data.jog_speedFactor = 2.7f;
        data.run_speedFactor = 3.2f;
        data.dash_speedFactor = 3.4f;
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

//---------------------------------------------------
// Main Variables
//---------------------------------------------------
bool NextGenBlaze::m_doubleJumpEnabled = true;
float const cBlaze_doubleJumpPower = 18.0f;

float NextGenBlaze::m_xHeldTimer = 0.0f;
float NextGenBlaze::m_homingVSpeed = 0.0f;
bool NextGenBlaze::m_dummyHoming = false;
float const cBlaze_fireTornadoMinTime = 0.3f;
float const cBlaze_dummyHomingSpeed = 20.0f;
float const cBlaze_dummyHomingVSpeedMax = 9.0f;
float const cBlaze_dummyHomingGravity = -10.0f;

float NextGenBlaze::m_spinningClawSpeed = 0.0f;
float NextGenBlaze::m_spinningClawTime = 0.0f;
float const cBlaze_spinningClawMinTime = 3.0f;
float const cBlaze_spinningClawMaxTime = 15.0f;

char const* ef_ch_bl_firetornado = "ef_ch_bl_firetornado";
char const* ef_ch_bl_axeljump = "ef_ch_bl_axeljump";

//-------------------------------------------------------
// Double Jump/Homing Attack
//-------------------------------------------------------
HOOK(int, __stdcall, NextGenBlaze_HomingUpdate, 0xE5FF10, CSonicContext* context)
{
    return originalNextGenBlaze_HomingUpdate(context);
}

HOOK(bool, __fastcall, NextGenBlaze_HomingAttackCheck, 0xDFFE30, CSonicContext* context, void* Edx, int a2)
{
    // Cannot use homing attack after double jump
    if (!NextGenBlaze::m_doubleJumpEnabled) return false;

    return originalNextGenBlaze_HomingAttackCheck(context, Edx, a2);
}

void __declspec(naked) NextGenBlaze_AirAction()
{
    static uint32_t returnAddress = 0xDFFEE5;
    static uint32_t fnButtonPress = 0xD97E00;
    __asm
    {
        // Homing attack/Fire tornado
        call    NextGenBlaze::bActionHandlerImpl
        test    al, al
        jnz     jump

        // Double jump
        mov     eax, [esi+11Ch]
        push    edi
        xor     edi, edi
        call    [fnButtonPress]
        pop     edi
        mov     ecx, eax
        call    NextGenBlaze::doubleJumpImpl

        jump:
        jmp     [returnAddress]
    }
}

bool NextGenBlaze::doubleJumpImpl(bool pressedJump)
{
    if (!m_doubleJumpEnabled) return false;

    if (pressedJump)
    {
        m_doubleJumpEnabled = false;

        // Set velocity
        Eigen::Vector3f velocity;
        Common::GetPlayerVelocity(velocity);
        velocity.y() = cBlaze_doubleJumpPower;
        if (Common::GetSonicStateFlags()->OnWater)
        {
            velocity.y() -= 5.0f;
        }
        Common::SetPlayerVelocity(velocity);

        // Play double jump voice and change animation
        static SharedPtrTypeless voiceHandle;
        Common::SonicContextPlaySound(voiceHandle, 3002030, 0);
        WRITE_MEMORY(0x1235263, char*, AnimationSetPatcher::AccelJumpLoop);

        // Play accel jump sfx
        static SharedPtrTypeless sfxHandle;
        Common::SonicContextPlaySound(sfxHandle, 80041023, 1);

        // Accel jump pfx
        WRITE_MEMORY(0x11BCDA7, char**, &ef_ch_bl_axeljump);
        WRITE_MEMORY(0x11BCCFC, char**, &ef_ch_bl_axeljump);

        // Change state
        Common::GetSonicStateFlags()->EnableAttenuateJump = true;
        StateManager::ChangeState(StateAction::Jump, *PLAYER_CONTEXT);

        // Resume disable lock-on cursor
        WRITE_MEMORY(0xDEBAA0, uint8_t, 0x75);
        return true;
    }

    return false;
}

HOOK(char*, __fastcall, NextGenBlaze_CSonicStateJumpBallBegin, 0x11BCBE0, void* This)
{
    char* result = originalNextGenBlaze_CSonicStateJumpBallBegin(This);
    {
        // Restore spin attack animation
        WRITE_MEMORY(0x1235263, uint32_t, 0x15F84F4);

        // Restore jump pfx (nothing)
        WRITE_MEMORY(0x11BCDA7, uint32_t, 0x1E61D6C);
        WRITE_MEMORY(0x11BCCFC, uint32_t, 0x1E61D00);
    }
    return result;
}

HOOK(void, __fastcall, NextGenBlaze_MsgSpringImpulse, 0xE6C3D0, void* This, void* Edx, MsgApplyImpulse* message)
{
    NextGenBlaze::m_doubleJumpEnabled = true;
    originalNextGenBlaze_MsgSpringImpulse(This, Edx, message);
}

HOOK(void, __fastcall, NextGenBlaze_MsgWideSpringImpulse, 0xE6C310, void* This, void* Edx, MsgApplyImpulse* message)
{
    NextGenBlaze::m_doubleJumpEnabled = true;
    originalNextGenBlaze_MsgWideSpringImpulse(This, Edx, message);
}

HOOK(void, __fastcall, NextGenBlaze_MsgApplyImpulse, 0xE6CFA0, void* This, void* Edx, MsgApplyImpulse* message)
{
    NextGenBlaze::m_doubleJumpEnabled = true;
    originalNextGenBlaze_MsgApplyImpulse(This, Edx, message);
}

HOOK(int*, __fastcall, NextGenBlaze_CSonicStateGrindBegin, 0xDF26A0, void* This)
{
    NextGenBlaze::m_doubleJumpEnabled = true;
    return originalNextGenBlaze_CSonicStateGrindBegin(This);
}

HOOK(void, __fastcall, NextGenBlaze_CSonicStateWallJumpReadyBegin, 0x123DA70, void* This)
{
    NextGenBlaze::m_doubleJumpEnabled = true;
    return originalNextGenBlaze_CSonicStateWallJumpReadyBegin(This);
}

HOOK(int, __fastcall, NextGenBlaze_CSonicStateHomingAttackBegin, 0x1232040, uint32_t** This)
{
    uint32_t* owner = This[2];
    if (!owner[0x3A6])
    {
        Eigen::Vector3f velocity;
        Common::GetPlayerVelocity(velocity);
        NextGenBlaze::m_homingVSpeed = min(velocity.y(), cBlaze_dummyHomingVSpeedMax);
        if (Common::IsPlayerGrounded())
        {
            NextGenBlaze::m_homingVSpeed = cBlaze_dummyHomingVSpeedMax;
        }

        // Don't stop by ground on dummy homing attack going up
        if (NextGenBlaze::m_homingVSpeed > 0.0f)
        {
            WRITE_MEMORY(0x1231E36, uint8_t, 0xEB, 0x5E);
        }

        // Set initial horizontal velocity
        Eigen::Vector3f position;
        Eigen::Quaternionf rotation;
        Common::GetPlayerTransform(position, rotation);
        Eigen::Vector3f hDir = rotation * Eigen::Vector3f(0, 0, 1);
        hDir.y() = 0.0f;
        hDir.normalize();
        velocity = hDir * cBlaze_dummyHomingSpeed;
        velocity.y() = NextGenBlaze::m_homingVSpeed;
        Common::SetPlayerVelocity(velocity);

        NextGenBlaze::m_dummyHoming = true;
    }
    else
    {
        // Don't stop by ground on homing attack
        WRITE_MEMORY(0x1231E36, uint8_t, 0xEB, 0x5E);
        NextGenBlaze::m_dummyHoming = false;
    }

    return originalNextGenBlaze_CSonicStateHomingAttackBegin(This);
}

HOOK(void, __fastcall, NextGenBlaze_CSonicStateHomingAttackAdvance, 0x1231C60, uint32_t** This)
{
    bool forceInAir = true;

    // Do not auto update rotation in the air
    WRITE_MEMORY(0xE32423, uint8_t, 0xEB);

    uint32_t* owner = This[2];
    if (NextGenBlaze::m_dummyHoming)
    {
        // No homing attack target, keep constant forward velocity
        Eigen::Vector3f velocity;
        Common::GetPlayerVelocity(velocity);
        Eigen::Vector3f hVel = velocity;
        hVel.y() = 0.0f;

        // Transition out if hVel is too low
        if (hVel.squaredNorm() < 100.0f)
        {
            StateManager::ChangeState(StateAction::Fall, owner);
            return;
        }

        // Change vVel base on pseudo gravity
        NextGenBlaze::m_homingVSpeed = max(NextGenBlaze::m_homingVSpeed, velocity.y());
        NextGenBlaze::m_homingVSpeed = min(NextGenBlaze::m_homingVSpeed, cBlaze_dummyHomingVSpeedMax);
        NextGenBlaze::m_homingVSpeed += cBlaze_dummyHomingGravity / 60.0f;

        // Resume stop by ground if we are falling down
        if (NextGenBlaze::m_homingVSpeed <= 0.0f)
        {
            WRITE_MEMORY(0x1231E36, uint8_t, 0x74, 0x17);
            forceInAir = false;
        }

        // Update velocity
        velocity = hVel.normalized() * cBlaze_dummyHomingSpeed;
        velocity.y() = NextGenBlaze::m_homingVSpeed;
        Common::SetPlayerVelocity(velocity);
    }

    // Force not on ground
    if (forceInAir)
    {
        *(bool*)((uint32_t)*PLAYER_CONTEXT + 0x440) = false;
        *(bool*)((uint32_t)*PLAYER_CONTEXT + 0x360) = false;
    }

    originalNextGenBlaze_CSonicStateHomingAttackAdvance(This);
}

HOOK(void, __fastcall, NextGenBlaze_CSonicStateHomingAttackEnd, 0x1231F80, uint32_t** This)
{
    // Resume auto update rotation in the air
    WRITE_MEMORY(0xE32423, uint8_t, 0x75);

    // Resume hit ground on homing attack
    WRITE_MEMORY(0x1231E36, uint8_t, 0x74, 0x17);

    originalNextGenBlaze_CSonicStateHomingAttackEnd(This);
}

//---------------------------------------------------
// X Button Action
//---------------------------------------------------
HOOK(bool, __stdcall, NextGenBlaze_BActionHandler, 0xDFF660, CSonicContext* context, bool buttonHoldCheck)
{
    // Reset double jump status
    NextGenBlaze::m_doubleJumpEnabled = true;

    // Handle X button action
    bool result = originalNextGenBlaze_BActionHandler(context, buttonHoldCheck);
    if (result || Common::IsPlayerControlLocked())
    {
        NextGenBlaze::m_xHeldTimer = 0.0f;
        return result;
    }

    // Force never disable lock-on cursor unless no target
    if (!Common::GetSonicStateFlags()->EnableGravityControl)
    {
        WRITE_MEMORY(0xDEBAA0, uint8_t, 0xEB);
        originalNextGenBlaze_HomingUpdate(context);
    }

    return NextGenBlaze::bActionHandlerImpl();
}

bool NextGenBlaze::bActionHandlerImpl()
{
    bool bDown, bPressed, bReleased;
    NextGenPhysics::getActionButtonStates(bDown, bPressed, bReleased);
    if (bDown)
    {
        // Remember how long we held X, always run in 60fps
        m_xHeldTimer += 1.0f / 60.0f;
    }
    else
    {
        bool success = false;
        if (bReleased)
        {
            if (m_xHeldTimer <= cBlaze_fireTornadoMinTime)
            {
                // Blaze always homing attack with X
                StateManager::ChangeState(StateAction::HomingAttack, *PLAYER_CONTEXT);
                success = true;
            }
            else if (!Common::GetSonicStateFlags()->OnWater)
            {
                // Held X, use fire tornado
                StateManager::ChangeState(StateAction::Sliding, *PLAYER_CONTEXT);
                success = true;
            }
        }

        m_xHeldTimer = 0.0f;

        if (success)
        {
            // Resume disable lock-on cursor
            WRITE_MEMORY(0xDEBAA0, uint8_t, 0x75);
            return true;
        }
    }

    return false;
}

//---------------------------------------------------
// CSonicStateSliding
//---------------------------------------------------
HOOK(int, __fastcall, NextGenBlaze_CSonicStateSlidingBegin, 0x11D7110, void* This)
{
    // Get current sliding speed
    Eigen::Vector3f playerVelocity;
    Common::GetPlayerVelocity(playerVelocity);
    playerVelocity.y() = 0.0f;
    NextGenBlaze::m_spinningClawSpeed = playerVelocity.norm();

    // In air uses longer time
    NextGenBlaze::m_spinningClawTime = Common::IsPlayerGrounded() ? cBlaze_spinningClawMinTime : cBlaze_spinningClawMaxTime;

    return originalNextGenBlaze_CSonicStateSlidingBegin(This);
}

HOOK(void, __fastcall, NextGenBlaze_CSonicStateSlidingAdvance, 0x11D69A0, int This)
{
    originalNextGenBlaze_CSonicStateSlidingAdvance(This);

    bool bDown, bPressed, bReleased;
    NextGenPhysics::getActionButtonStates(bDown, bPressed, bReleased);

    if (bPressed && NextGenPhysics::checkUseLightSpeedDash())
    {
        // Pressed X button and use light speed dash
        return;
    }

    float stateTime = *(float*)(This + 16);
    if (bReleased || stateTime > NextGenBlaze::m_spinningClawTime)
    {
        // Cancel sliding
        StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
        return;
    }
}

HOOK(void, __fastcall, NextGenBlaze_CSonicStateSlidingEnd, 0x11D7010, int This)
{
    // Play spinning claw stop stx
    static SharedPtrTypeless soundHandle;
    Common::SonicContextPlaySound(soundHandle, 80041022, 1);

    originalNextGenBlaze_CSonicStateSlidingEnd(This);
}

HOOK(int, __fastcall, NextGenBlaze_CSonicPosture2DSliding, 0x11D9720, int This)
{
    int result = originalNextGenBlaze_CSonicPosture2DSliding(This);
    {
        NextGenPhysics::keepConstantHorizontalVelocity(NextGenBlaze::m_spinningClawSpeed);
    }
    return result;
}

HOOK(int, __fastcall, NextGenBlaze_CSonicPosture3DSliding, 0x11D93B0, int This)
{
    int result = originalNextGenBlaze_CSonicPosture3DSliding(This);
    {
        NextGenPhysics::keepConstantHorizontalVelocity(NextGenBlaze::m_spinningClawSpeed);
    }
    return result;
}

//---------------------------------------------------
// Main Apply Patches
//---------------------------------------------------
void NextGenBlaze::applyPatches()
{
    // Stomp uses fire tornado voice & sfx
    WRITE_MEMORY(0x1254E02, uint8_t, 10);
    WRITE_MEMORY(0x1254E04, uint32_t, 3002031);
    WRITE_MEMORY(0x1254E73, char**, &ef_ch_bl_firetornado);
    WRITE_MEMORY(0x1254E9A, char**, &ef_ch_bl_firetornado);
    WRITE_MEMORY(0x12548FC, uint32_t, 80041022);
    WRITE_MEMORY(0x12549D8, uint32_t, 80041022);

    // Replace homing attack animation with air boost
    WRITE_MEMORY(0x1232056, char*, "AirBoost");

    // Don't use any lotus effect (homing attack/stomp)
    WRITE_JUMP(0xE5FDEA, (void*)0xE5FF03);

    // Fire tornado voice & sfx
    WRITE_MEMORY(0x11D722C, uint32_t, 2002042);
    WRITE_MEMORY(0x11D72DA, uint8_t, 10);
    WRITE_MEMORY(0x11D72DC, uint32_t, 3002031);
    INSTALL_HOOK(NextGenBlaze_CSonicStateSlidingEnd);

    // Fire tornado animation
    WRITE_MEMORY(0x11D7124, char*, AnimationSetPatcher::FireTornadoLoop);
    WRITE_MEMORY(0x11D6E6A, char*, AnimationSetPatcher::FireTornadoLoop);
    WRITE_MEMORY(0x11D6EDB, char*, AnimationSetPatcher::FireTornadoLoop);
    WRITE_MEMORY(0x1230F88, char*, AnimationSetPatcher::FireTornadoEnd);

    // Fire tornado pfx
    WRITE_MEMORY(0x11D6A0A, char**, &ef_ch_bl_firetornado);
    WRITE_MEMORY(0x11D6A80, char**, &ef_ch_bl_firetornado);

    if (!Configuration::m_characterMoveset) return;

    //-------------------------------------------------------
    // Double Jump/Homing Attack
    //-------------------------------------------------------
    INSTALL_HOOK(NextGenBlaze_HomingAttackCheck);
    WRITE_JUMP(0xDFFEA3, NextGenBlaze_AirAction);
    INSTALL_HOOK(NextGenBlaze_CSonicStateJumpBallBegin);

    // States that resets double jump
    INSTALL_HOOK(NextGenBlaze_MsgSpringImpulse);
    INSTALL_HOOK(NextGenBlaze_MsgWideSpringImpulse);
    INSTALL_HOOK(NextGenBlaze_MsgApplyImpulse);
    INSTALL_HOOK(NextGenBlaze_CSonicStateGrindBegin);
    INSTALL_HOOK(NextGenBlaze_CSonicStateWallJumpReadyBegin);

    // Homing Attack
    WRITE_JUMP(0x12321F4, (void*)0x1232383); // don't set initial dummy homing attack velocity
    WRITE_NOP(0x123551E, 6); // skip jumpball min time to fall
    INSTALL_HOOK(NextGenBlaze_CSonicStateHomingAttackBegin);
    INSTALL_HOOK(NextGenBlaze_CSonicStateHomingAttackAdvance);
    INSTALL_HOOK(NextGenBlaze_CSonicStateHomingAttackEnd);

    // Disable stomping
    WRITE_MEMORY(0xDFDDB3, uint8_t, 0xEB);

    //-------------------------------------------------------
    // X-Action State handling
    //-------------------------------------------------------
    // Return 0 for Squat and Sliding, handle them ourselves
    WRITE_MEMORY(0xDFF8D5, uint8_t, 0xEB, 0x05);
    WRITE_MEMORY(0xDFF856, uint8_t, 0xE9, 0x81, 0x00, 0x00, 0x00);
    INSTALL_HOOK(NextGenBlaze_BActionHandler);

    //-------------------------------------------------------
    // Fire Tornado
    //-------------------------------------------------------
    // Change slide to hit enemy as if you're squat kicking (TypeSonicSquatKick)
    WRITE_MEMORY(0x11D72F3, uint32_t, SonicCollision::TypeSonicSquatKick);
    WRITE_MEMORY(0x11D7090, uint32_t, SonicCollision::TypeSonicSquatKick);

    // Disable all Sliding transition out, handle them outselves
    WRITE_MEMORY(0x11D6B7D, uint8_t, 0xEB);
    WRITE_MEMORY(0x11D6C6D, uint8_t, 0xEB);
    WRITE_MEMORY(0x11D6CA2, uint8_t, 0xEB);
    WRITE_MEMORY(0x11D6F82, uint8_t, 0x90, 0xE9);
    WRITE_NOP(0x11D6F1A, 6);

    // Allow sliding have precise turn in air
    WRITE_NOP(0x11D943D, 2);

    // Spinning Claw
    INSTALL_HOOK(NextGenBlaze_CSonicStateSlidingBegin);
    INSTALL_HOOK(NextGenBlaze_CSonicStateSlidingAdvance);

    // Set constant sliding speed
    INSTALL_HOOK(NextGenBlaze_CSonicPosture2DSliding);
    INSTALL_HOOK(NextGenBlaze_CSonicPosture3DSliding);

    // Do not use sliding fail pfx
    WRITE_NOP(0x11D6A6D, 2);

    // Change "Stomping" type object physics to "Normal"
    WRITE_MEMORY(0xE9FFC9, uint32_t, 5);
}
