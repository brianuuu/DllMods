#include "CustomCamera.h"
#include "Application.h"
#include "Configuration.h"

bool m_blockByTerrainMode = false;
float const c_pitchMin = -20.0f * DEG_TO_RAD;
float const c_pitchMax = 50.0f * DEG_TO_RAD;
float const c_pitchDefault = 6.0f * DEG_TO_RAD;
float const c_pitchCorrection = 2.0f * DEG_TO_RAD;
float const c_pitchCorrectionMachSpeed = c_pitchCorrection + 5.0f * DEG_TO_RAD;
float const c_pitchDistanceUp = 20.0f * DEG_TO_RAD;
float const c_pitchDistanceUpScale = 1.2f;
float const c_pitchDistanceDown = 1.0f * DEG_TO_RAD;
float const c_pitchDistanceDownScale = 0.03f;
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
float targetCameraToPlayerDist = c_cameraToPlayerDistMin;
bool m_wasPaused = false;
bool m_usedCustomCamera = false;
int m_usedCustomCameraLastFrame = false;
Eigen::Vector3f cameraPosCached(0, 0, 0);
Eigen::Vector3f cameraPosPitchCorrected(0, 0, 0);
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

    // Use default camera on quick step path
    if (Common::IsPlayerInForwardPath())
    {
        cameraPosCached = *pCameraPos;
        return result;
    }

    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    Common::GetPlayerTransform(playerPosition, playerRotation);
    playerPosition += playerRotation * Eigen::Vector3f(0, c_cameraTargetOffset, 0);

    Eigen::Vector3f playerVelocity;
    Common::GetPlayerVelocity(playerVelocity);
    CSonicStateFlags* flags = Common::GetSonicStateFlags();
    Sonic::SPadState* padState = Sonic::CInputState::GetPadState();

    float const dt = 1.0f / 60.0f; // this always run at 60fps
    bool isReset = !Common::IsPlayerControlLocked() && padState->IsTapped(Sonic::EKeyState::eKeyState_LeftTrigger);

    // Calculate current pitch correction
    float pitchCorrection = flags->KeepRunning ? c_pitchCorrectionMachSpeed : c_pitchCorrection;
    targetPitchCorrection += (pitchCorrection - targetPitchCorrection) * c_cameraLerpRate * dt;
    if (!m_usedCustomCameraLastFrame)
    {
        targetPitchCorrection = pitchCorrection;
    }
    float const pitchMin = c_pitchMin - targetPitchCorrection;
    float const pitchMax = c_pitchMax - targetPitchCorrection;

    // Get currect camera direction from player
    if (!m_usedCustomCameraLastFrame)
    {
        cameraPosCached = *pCameraPos;
    }
    *pCameraTarget = playerPosition;
    if (!Common::IsPlayerGrounded())
    {
        cameraPosCached += Eigen::Vector3f::UnitY() * playerVelocity.y() * (playerVelocity.y() > 0.0f ? c_cameraInAirVelocitySensitivePositive : c_cameraInAirVelocitySensitiveNegative) * dt;
    }
    Eigen::Vector3f dir = (cameraPosCached - playerPosition).normalized();
    if (isReset)
    {
        dir = -(playerRotation * Eigen::Vector3f::UnitZ());
    }
    
    // Get and smooth current player rotation
    targetPlayerRotation = targetPlayerRotation.slerp(c_cameraRotateRate * dt, Common::IsPlayerGrounded() ? playerRotation : Eigen::Quaternionf(1, 0, 0, 0));
    if (Common::IsAtLoadingScreen())
    {
        targetPlayerRotation = Eigen::Quaternionf(1, 0, 0, 0);
    }
    Eigen::Vector3f playerUpAxis = targetPlayerRotation * Eigen::Vector3f::UnitY();
    //printf("%.3f, %.3f, %.3f\n", playerUpAxis.x(), playerUpAxis.y(), playerUpAxis.z());
        
    // Apply yaw
    float const invertX = Configuration::m_cameraInvertX ? -1.0f : 1.0f;
    float const cameraYawAdd = Common::IsPlayerControlLocked() ? 0.0f : padState->RightStickHorizontal * c_cameraRotateRate * dt * invertX;
    targetYawAdd += (cameraYawAdd - targetYawAdd) * c_cameraLerpRate * dt;
    Common::ClampFloat(targetYawAdd, -0.5f * PI_F, 0.5f * PI_F);
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
        float const invertY = Configuration::m_cameraInvertY ? -1.0f : 1.0f;
        float const pitchAdd = Common::IsPlayerControlLocked() ? 0.0f : -(padState->RightStickVertical) * c_cameraRotateRate * dt * invertY;
        pitch += pitchAdd * (pitchAdd > 0.0f ? 1.5f : 1.0f);
    }
    Common::ClampFloat(pitch, pitchMin, pitchMax);
    
    // Counteract pitch correction if we didn't do it last frame
    // Yes, it's very hacky I know, not the best solution since it might still move a little after pausing
    if (!m_usedCustomCameraLastFrame)
    {
        pitch -= targetPitchCorrection;
    }
    targetPitch += (pitch + targetPitchCorrection - targetPitch) * c_cameraLerpRate * dt;
    Common::ClampFloat(targetPitch, c_pitchMin, c_pitchMax);

    // Calculate current target camera distance
    float playerSpeed = playerVelocity.norm();
    float speedDistAdd = (c_cameraToPlayerDistMax - c_cameraToPlayerDistMin) * playerSpeed / 20.0f;
    float cameraToPlayerDist = c_cameraToPlayerDistMin + speedDistAdd;
    Common::ClampFloat(cameraToPlayerDist, c_cameraToPlayerDistMin, c_cameraToPlayerDistMax);

    // Override distance when auto run or on board
    if (flags->KeepRunning || Common::IsPlayerOnBoard())
    {
        cameraToPlayerDist = c_cameraToPlayerDistFixed;
    }

    // Scale camera distance base on pitch
    if (targetPitch > c_pitchDistanceUp)
    {
        cameraToPlayerDist *= 1.0f + (c_pitchDistanceUpScale - 1.0f) * (targetPitch - c_pitchDistanceUp);
    }

    // Interpolate target camera distance
    targetCameraToPlayerDist += (cameraToPlayerDist - targetCameraToPlayerDist) * c_cameraLerpRate * dt;
    Common::ClampFloat(targetCameraToPlayerDist, 2.0f, 30.0f);

    // Get camera pos before/after pitch correction
    dir = CustomCamera::calculateCameraPos(playerPosition, playerUpAxis, pitchAxis, pitch, targetPitch);

    // Use raycast to prevent camera clipping through terrain
    Eigen::Vector4f const rayStartPos(playerPosition.x(), playerPosition.y() - 0.1f, playerPosition.z(), 1.0f);
    Eigen::Vector4f const rayEndPos(cameraPosPitchCorrected.x(), cameraPosPitchCorrected.y() - 0.1f, cameraPosPitchCorrected.z(), 1.0f);
    Eigen::Vector4f outPos;
    Eigen::Vector4f outNormal;
    if (Common::fRaycast(rayStartPos, rayEndPos, outPos, outNormal, 21))
    {
        //printf("%.3f, %.3f, %.3f\n", outPos.x(), outPos.y(), outPos.z());
        if (!m_blockByTerrainMode)
        {
            // Just zoom closer
            targetCameraToPlayerDist = (outPos.head<3>() - playerPosition).norm();
            CustomCamera::calculateCameraPos(playerPosition, playerUpAxis, pitchAxis, pitch, targetPitch);
        }
        else
        {
            // For 06, it blocks and pushes away camera
            // TODO: I don't know how to do this yet
        }
    }

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
    if (Configuration::m_camera)
    {
        // Override BOOST_BEGIN_FOVY_TARGET and BOOST_FOVY_SPS_BEGIN
        static float const BOOST_BEGIN_FOVY_TARGET = 75.0f;
        WRITE_MEMORY(0x10E81FC, float*, &BOOST_BEGIN_FOVY_TARGET);
        WRITE_MEMORY(0x10E81F2, float*, &BOOST_BEGIN_FOVY_TARGET);

        // Override BOOST_LOOP_FOVY_TARGET and BOOST_FOVY_SPS
        static float const BOOST_LOOP_FOVY_TARGET = 60.0f;
        WRITE_MEMORY(0x10E7BA6, float*, &BOOST_LOOP_FOVY_TARGET);
        WRITE_MEMORY(0x10E7B9C, float*, &BOOST_LOOP_FOVY_TARGET);

        INSTALL_HOOK(CPlayer3DNormalCameraAdvance);
        INSTALL_HOOK(CustomCamera_MsgStartPause);
        INSTALL_HOOK(CustomCamera_MsgFinishPause);
    }

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
        if (m_usedCustomCamera)
        {
            m_usedCustomCameraLastFrame = 2;
        }
        else if (m_usedCustomCameraLastFrame > 0)
        {
            m_usedCustomCameraLastFrame--;
        }
    }
    m_usedCustomCamera = false;
}

Eigen::Vector3f CustomCamera::calculateCameraPos
(
    Eigen::Vector3f const& playerPosition,
    Eigen::Vector3f const& playerUpAxis,
    Eigen::Vector3f const& pitchAxis,
    float const& pitch,
    float const& targetPitch
)
{
    Eigen::Vector3f dir;

    // Pitch before correction
    Eigen::Quaternionf rotationPitch(0, 0, 0, 1);
    rotationPitch = Eigen::AngleAxisf(90.0f * DEG_TO_RAD - pitch, pitchAxis);
    dir = rotationPitch * playerUpAxis;
    cameraPosCached = playerPosition + dir * targetCameraToPlayerDist;

    // Apply pitch correction
    Eigen::Quaternionf rotationPitchAdd(0, 0, 0, 1);
    rotationPitchAdd = Eigen::AngleAxisf(90.0f * DEG_TO_RAD - targetPitch, pitchAxis);
    dir = rotationPitchAdd * playerUpAxis;

    Eigen::Vector3f cameraPosPrev = cameraPosPitchCorrected;
    cameraPosPitchCorrected = playerPosition + dir * targetCameraToPlayerDist;
    //printf("dist = %.3f, pitch = %.3f, pitch correction = %.3f\n", targetCameraToPlayerDist, targetPitch * RAD_TO_DEG, targetPitchCorrection * RAD_TO_DEG);

    return dir;
}
