#include "CustomCamera.h"
#include "Application.h"
#include "Configuration.h"

float WrapFloat(float number, float bounds)
{
    if (number > bounds) number -= bounds;
    if (number < 0) number += bounds;
    return number;
}

void ClampFloat(float& number, float min, float max)
{
    if (number < min) number = min;
    if (number > max) number = max;
}

bool IsLoadingScreen()
{
    uint32_t** hudCount = (uint32_t**)0x1E66B40;
    if (!*hudCount) return false;
    return (*hudCount)[2] > 0;
}

float const c_pitchDefault = 6.0f * DEG_TO_RAD;
float const c_pitchCorrection = 10.0f * DEG_TO_RAD;
float const c_pitchCorrectionMachSpeed = 15.0f * DEG_TO_RAD;
float const c_cameraRotateRate = 100.0f * DEG_TO_RAD;
float const c_cameraLerpRate = 10.0f;
float const c_cameraToPlayerDistFixed = 6.0f;
float const c_cameraToPlayerDistMin = 6.5f;
float const c_cameraToPlayerDistMax = 8.0f;
float const c_cameraTargetOffset = 1.05f;
float const c_cameraInAirVelocitySensitivePositive = 0.5f;
float const c_cameraInAirVelocitySensitiveNegative = 0.2f;
float targetYawAdd = 0.0f;
float targetPitch = 0.0f;
float targetPitchCorrection = c_pitchCorrection;
float targetCameraToPlayerDist = 0.0f;
bool m_wasPaused = false;
bool m_usedCustomCamera = false;
bool m_usedCustomCameraLastFrame = false;
Eigen::Vector3f cameraPosCached(0, 0, 0);
Eigen::Quaternionf targetPlayerRotation(1, 0, 0, 0);
HOOK(int, __fastcall, CPlayer3DNormalCameraAdvance, 0x010EC7E0, int* This)
{
    uint32_t offset = (uint32_t)This;

    Eigen::Vector3f* pCameraPosVisual = (Eigen::Vector3f*)(offset + 0x70);
    Eigen::Vector3f* pCameraPosInputRef = (Eigen::Vector3f*)(offset + 0xA0);
    Eigen::Vector3f* pCameraPos = (Eigen::Vector3f*)(offset + 0x140);
    Eigen::Vector3f* pCameraTarget = (Eigen::Vector3f*)(offset + 0x90);
    Eigen::Vector3f* inputUnknown2 = (Eigen::Vector3f*)(offset + 0xC0);
    Eigen::Vector3f* vec6 = (Eigen::Vector3f*)(offset + 0x150);

    int result = originalCPlayer3DNormalCameraAdvance(This);
    if (!*PLAYER_CONTEXT) return result;

    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    Common::GetPlayerTransform(playerPosition, playerRotation);
    Eigen::Vector3f playerVelocity;
    Common::GetPlayerVelocity(playerVelocity);
    CSonicStateFlags* flags = Common::GetSonicStateFlags();
    Sonic::SPadState* padState = Sonic::CInputState::GetPadState();

    float dt = Application::getDeltaTime();
    bool isReset = padState->IsTapped(Sonic::EKeyState::eKeyState_LeftTrigger);

    // Calculate current target camera distance
    float playerSpeed = playerVelocity.norm();
    float speedDistAdd = (c_cameraToPlayerDistMax - c_cameraToPlayerDistMin) * playerSpeed / 20.0f;
    float cameraToPlayerDist = min(c_cameraToPlayerDistMax, c_cameraToPlayerDistMin + speedDistAdd);
    bool distOverride = flags->KeepRunning || Common::IsPlayerOnBoard();
    if (distOverride)
    {
        cameraToPlayerDist = c_cameraToPlayerDistFixed;
    }
    targetCameraToPlayerDist += (cameraToPlayerDist - targetCameraToPlayerDist) * c_cameraLerpRate * dt;
    ClampFloat(targetCameraToPlayerDist, distOverride ? c_cameraToPlayerDistFixed : c_cameraToPlayerDistMin, c_cameraToPlayerDistMax);

    // Calculate current pitch correction
    float pitchCorrection = flags->KeepRunning ? c_pitchCorrectionMachSpeed : c_pitchCorrection;
    targetPitchCorrection += (pitchCorrection - targetPitchCorrection) * c_cameraLerpRate * dt;
    if (!m_usedCustomCameraLastFrame)
    {
        targetPitchCorrection = c_pitchCorrection;
    }
    float const pitchMin = -25.0f * DEG_TO_RAD - targetPitchCorrection;
    float const pitchMax = 60.0f * DEG_TO_RAD - targetPitchCorrection;

    // Get currect camera direction from player
    if (!m_usedCustomCameraLastFrame)
    {
        cameraPosCached = *pCameraPos;
    }
    *pCameraTarget = playerPosition + playerRotation * Eigen::Vector3f(0, c_cameraTargetOffset, 0);
    if (!Common::IsPlayerInGrounded())
    {
        cameraPosCached += Eigen::Vector3f::UnitY() * playerVelocity.y() * (playerVelocity.y() > 0.0f ? c_cameraInAirVelocitySensitivePositive : c_cameraInAirVelocitySensitiveNegative) * dt;
    }
    Eigen::Vector3f dir = (cameraPosCached - playerPosition).normalized();
    if (isReset)
    {
        dir = -(playerRotation * Eigen::Vector3f::UnitZ());
    }
    
    // Get and smooth current player rotation
    targetPlayerRotation = targetPlayerRotation.slerp(c_cameraRotateRate * dt, Common::IsPlayerInGrounded() ? playerRotation : Eigen::Quaternionf(1, 0, 0, 0));
    if (IsLoadingScreen())
    {
        targetPlayerRotation = Eigen::Quaternionf(1, 0, 0, 0);
    }
    Eigen::Vector3f playerUpAxis = targetPlayerRotation * Eigen::Vector3f::UnitY();
    //printf("%.3f, %.3f, %.3f\n", playerUpAxis.x(), playerUpAxis.y(), playerUpAxis.z());
        
    // Apply yaw
    float const cameraYawAdd = padState->RightStickHorizontal * c_cameraRotateRate * dt;
    targetYawAdd += (cameraYawAdd - targetYawAdd) * c_cameraLerpRate * dt;
    Eigen::Quaternionf rotationYaw(0, 0, 0, 1);
    rotationYaw = Eigen::AngleAxisf(targetYawAdd, playerUpAxis);
    dir = rotationYaw * dir;

    // Apply pitch
    Eigen::Vector3f pitchAxis = playerUpAxis.cross(dir);
    if (pitchAxis.isZero())
    {
        pitchAxis = Eigen::Vector3f::UnitZ();
    }
    pitchAxis.normalize();
    float pitch = 90.0f * DEG_TO_RAD - acos(dir.dot(playerUpAxis));
    if (isReset)
    {
        pitch = c_pitchDefault;
    }
    else
    {
        pitch += -(padState->RightStickVertical) * c_cameraRotateRate * dt;
    }
    ClampFloat(pitch, pitchMin, pitchMax);
    
    // Counteract pitch correction if we didn't do it last frame
    // Yes, it's very hacky I know, not the best solution since it might still move a little after pausing
    if (!m_usedCustomCameraLastFrame)
    {
        pitch -= targetPitchCorrection;
    }

    // Pitch before correction
    Eigen::Quaternionf rotationPitch(0, 0, 0, 1);
    rotationPitch = Eigen::AngleAxisf(90.0f * DEG_TO_RAD - pitch, pitchAxis);
    dir = rotationPitch * playerUpAxis;
    cameraPosCached = playerPosition + dir * targetCameraToPlayerDist;

    // Apply pitch correction
    pitch += targetPitchCorrection;
    targetPitch += (pitch - targetPitch) * c_cameraLerpRate * dt;
    Eigen::Quaternionf rotationPitchAdd(0, 0, 0, 1);
    rotationPitchAdd = Eigen::AngleAxisf(90.0f * DEG_TO_RAD - targetPitch, pitchAxis);
    dir = rotationPitchAdd * playerUpAxis;
    Eigen::Vector3f cameraPosPitchCorrected = playerPosition + dir * targetCameraToPlayerDist;
    //printf("dist = %.3f, pitch = %.3f, pitch correction = %.3f\n", targetCameraToPlayerDist, targetPitch * RAD_TO_DEG, targetPitchCorrection * RAD_TO_DEG);

    // Apply final rotation
    Eigen::Quaternionf rotation = Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f::UnitZ(), dir);
    *pCameraPos = cameraPosPitchCorrected;
    *pCameraPosVisual = cameraPosPitchCorrected;
    *pCameraPosInputRef = Eigen::Vector3f::Zero();
    *inputUnknown2 = -dir;

    m_usedCustomCamera = true;
    return result;
}

HOOK(bool, __fastcall, CustomCamera_MsgStartPause, 0x010BC130, void* This, void* Edx, void* a2)
{
    m_wasPaused = true;
    return originalCustomCamera_MsgStartPause(This, Edx, a2);
}

HOOK(int, __fastcall, CustomCamera_MsgFinishPause, 0x010BC110, void* This, void* Edx, void* a2)
{
    m_wasPaused = false;
    return originalCustomCamera_MsgFinishPause(This, Edx, a2);
}

void CustomCamera::applyPatches()
{
    // Override BOOST_BEGIN_FOVY_TARGET
    static float const BOOST_BEGIN_FOVY_TARGET = 75.0f;
    WRITE_MEMORY(0x10E81FC, float*, &BOOST_BEGIN_FOVY_TARGET);

    // Override BOOST_LOOP_FOVY_TARGET
    static float const BOOST_LOOP_FOVY_TARGET = 60.0f;
    WRITE_MEMORY(0x10E7BA6, float*, &BOOST_LOOP_FOVY_TARGET);

    INSTALL_HOOK(CPlayer3DNormalCameraAdvance);
    INSTALL_HOOK(CustomCamera_MsgStartPause);
    INSTALL_HOOK(CustomCamera_MsgFinishPause);

    if (Configuration::m_physics)
    {
        // Override HOMING_ATTACK_TARGET_SENSITIVE
        static float const HOMING_ATTACK_TARGET_SENSITIVE = 0.0f;
        WRITE_MEMORY(0x10EEB92, float*, &HOMING_ATTACK_TARGET_SENSITIVE);
    }
}

void CustomCamera::advance()
{
    if (!m_wasPaused)
    {
        m_usedCustomCameraLastFrame = m_usedCustomCamera;
    }
    m_usedCustomCamera = false;
}