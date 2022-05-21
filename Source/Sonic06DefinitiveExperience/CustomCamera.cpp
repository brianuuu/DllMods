#include "CustomCamera.h"
#include "Application.h"
#include "Configuration.h"
#include "NextGenSonic.h"

// Normal Camera
bool m_blockByTerrainMode = false;
float const cCamera_pitchMin = -20.0f * DEG_TO_RAD;
float const cCamera_pitchMax = 50.0f * DEG_TO_RAD;
float const cCamera_pitchDefault = 6.0f * DEG_TO_RAD;
float const cCamera_pitchCorrection = 2.0f * DEG_TO_RAD;
float const cCamera_pitchCorrectionMachSpeed = cCamera_pitchCorrection + 5.0f * DEG_TO_RAD;
float const cCamera_pitchCorrectionPurpleGem = cCamera_pitchCorrection + 5.0f * DEG_TO_RAD;
float const cCamera_pitchDistanceUp = 20.0f * DEG_TO_RAD;
float const cCamera_pitchDistanceUpScale = 1.2f;
float const cCamera_pitchDistanceDown = 1.0f * DEG_TO_RAD;
float const cCamera_pitchDistanceDownScale = 0.03f;
float const cCamera_rotateRate = 100.0f * DEG_TO_RAD;
float const cCamera_lerpRate = 10.0f;
float const cCamera_toPlayerDistFixed = 6.0f;
float const cCamera_toPlayerDistPurpleGem = 1.3f;
float const cCamera_toPlayerDistMin = 6.5f;
float const cCamera_toPlayerDistMax = 8.0f;
float const cCamera_targetOffset = 1.05f;
float const cCamera_targetOffsetPurpleGem = 0.45f;
float const cCamera_inAirVelocitySensitivePositive = 0.5f;
float const cCamera_inAirVelocitySensitiveNegative = 0.2f;
float targetYawAdd = 0.0f;
float targetPitch = 0.0f;
float targetPitchCorrection = cCamera_pitchCorrection;
float targetCameraToPlayerDist = cCamera_toPlayerDistMin;
bool m_wasPaused = false;
bool m_usedCustomCamera = false;
int m_usedCustomCameraLastFrame = false;
Eigen::Vector3f cameraPosCached(0, 0, 0);
Eigen::Vector3f cameraPosPitchCorrected(0, 0, 0);
Eigen::Quaternionf targetPlayerRotation(1, 0, 0, 0);

// Sky Gem
float const cCamera_skyGemToPlayerDist = 1.3f;
float const cCamera_skyGemtargetOffset = 0.95f;
float const cCamera_skyGemRotateRate = 50.0f * DEG_TO_RAD;
float const cCamera_skyGemPitchMax = 50.0f * DEG_TO_RAD;
float const cCamera_skyGemPitchMin = -50.0f * DEG_TO_RAD;
float const cCamera_skyGemFactorRate = 3.0f;
float CustomCamera::m_skyGemCameraPitch = 0.0f;
float m_skyGemCameraFactor = 0.0f;
bool m_skyGemCameraUpdated = false;
Hedgehog::Math::CVector m_skyGemCameraReleasedPosition;
Hedgehog::Math::CQuaternion m_skyGemCameraReleasedRotation;

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

    float targetOffset = NextGenSonic::m_purpleGemEnabled ? cCamera_targetOffsetPurpleGem : cCamera_targetOffset;
    if (Common::IsPlayerGrounded())
    {
        playerPosition += playerRotation * Eigen::Vector3f(0, targetOffset, 0);
    }
    else
    {
        playerPosition += Eigen::Vector3f(0, targetOffset, 0);
    }

    Eigen::Vector3f playerVelocity;
    Common::GetPlayerVelocity(playerVelocity);
    CSonicStateFlags* flags = Common::GetSonicStateFlags();
    Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();

    float const dt = 1.0f / 60.0f; // this always run at 60fps
    bool isReset = !Common::IsPlayerControlLocked() && padState->IsTapped(Sonic::EKeyState::eKeyState_LeftTrigger);
    isReset |= NextGenSonic::m_skyGemEnabled && !NextGenSonic::m_skyGemLaunched && m_skyGemCameraFactor >= 1.0f;

    // Calculate current pitch correction
    float pitchCorrection = cCamera_pitchCorrection;
    if (NextGenSonic::m_purpleGemEnabled)
    {
        pitchCorrection = cCamera_pitchCorrectionPurpleGem;
    }
    else if (flags->KeepRunning)
    {
        pitchCorrection = cCamera_pitchCorrectionMachSpeed;
    }

    targetPitchCorrection += (pitchCorrection - targetPitchCorrection) * cCamera_lerpRate * dt;
    if (!m_usedCustomCameraLastFrame)
    {
        targetPitchCorrection = pitchCorrection;
    }
    float const pitchMin = cCamera_pitchMin - targetPitchCorrection;
    float const pitchMax = cCamera_pitchMax - targetPitchCorrection;

    // Get currect camera direction from player
    if (!m_usedCustomCameraLastFrame)
    {
        cameraPosCached = *pCameraPos;
    }
    *pCameraTarget = playerPosition;
    if (!Common::IsPlayerGrounded())
    {
        cameraPosCached += Eigen::Vector3f::UnitY() * playerVelocity.y() * (playerVelocity.y() > 0.0f ? cCamera_inAirVelocitySensitivePositive : cCamera_inAirVelocitySensitiveNegative) * dt;
    }
    Eigen::Vector3f dir = (cameraPosCached - playerPosition).normalized();
    if (isReset)
    {
        dir = -(playerRotation * Eigen::Vector3f::UnitZ());
    }
    
    // Get and smooth current player rotation
    targetPlayerRotation = targetPlayerRotation.slerp(cCamera_rotateRate * dt, Common::IsPlayerGrounded() ? playerRotation : Eigen::Quaternionf(1, 0, 0, 0));
    if (Common::IsAtLoadingScreen())
    {
        targetPlayerRotation = Eigen::Quaternionf(1, 0, 0, 0);
    }
    Eigen::Vector3f playerUpAxis = targetPlayerRotation * Eigen::Vector3f::UnitY();
    //printf("%.3f, %.3f, %.3f\n", playerUpAxis.x(), playerUpAxis.y(), playerUpAxis.z());
        
    // Apply yaw
    float const invertX = Configuration::m_cameraInvertX ? -1.0f : 1.0f;
    float const cameraYawAdd = Common::IsPlayerControlLocked() ? 0.0f : padState->RightStickHorizontal * cCamera_rotateRate * dt * invertX;
    targetYawAdd += (cameraYawAdd - targetYawAdd) * cCamera_lerpRate * dt;
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
        pitch = cCamera_pitchDefault;
    }
    else
    {
        float const invertY = Configuration::m_cameraInvertY ? -1.0f : 1.0f;
        float const pitchAdd = Common::IsPlayerControlLocked() ? 0.0f : -(padState->RightStickVertical) * cCamera_rotateRate * dt * invertY;
        pitch += pitchAdd * (pitchAdd > 0.0f ? 1.5f : 1.0f);
    }
    Common::ClampFloat(pitch, pitchMin, pitchMax);
    
    // Counteract pitch correction if we didn't do it last frame
    // Yes, it's very hacky I know, not the best solution since it might still move a little after pausing
    if (!m_usedCustomCameraLastFrame)
    {
        pitch -= targetPitchCorrection;
    }
    targetPitch += (pitch + targetPitchCorrection - targetPitch) * cCamera_lerpRate * dt;
    Common::ClampFloat(targetPitch, cCamera_pitchMin, cCamera_pitchMax);

    // Calculate current target camera distance
    float playerSpeed = playerVelocity.norm();
    float speedDistAdd = (cCamera_toPlayerDistMax - cCamera_toPlayerDistMin) * playerSpeed / 20.0f;
    float cameraToPlayerDist = cCamera_toPlayerDistMin + speedDistAdd;
    Common::ClampFloat(cameraToPlayerDist, cCamera_toPlayerDistMin, cCamera_toPlayerDistMax);

    // Override distance when auto run or on board
    if (NextGenSonic::m_purpleGemEnabled)
    {
        cameraToPlayerDist = cCamera_toPlayerDistPurpleGem;
    }
    else if (flags->KeepRunning || Common::IsPlayerOnBoard())
    {
        cameraToPlayerDist = cCamera_toPlayerDistFixed;
    }

    // Scale camera distance base on pitch
    if (targetPitch > cCamera_pitchDistanceUp)
    {
        cameraToPlayerDist *= 1.0f + (cCamera_pitchDistanceUpScale - 1.0f) * (targetPitch - cCamera_pitchDistanceUp);
    }

    // Interpolate target camera distance
    targetCameraToPlayerDist += (cameraToPlayerDist - targetCameraToPlayerDist) * cCamera_lerpRate * dt;
    Common::ClampFloat(targetCameraToPlayerDist, NextGenSonic::m_purpleGemEnabled ? 1.0f : 2.0f, 30.0f);

    // Get camera pos before/after pitch correction
    dir = CustomCamera::calculateCameraPos(playerPosition, playerUpAxis, pitchAxis, pitch, targetPitch);

    // Use raycast to prevent camera clipping through terrain
    Eigen::Vector4f const rayStartPos(playerPosition.x(), playerPosition.y() - 0.1f, playerPosition.z(), 1.0f);
    Eigen::Vector4f const rayEndPos(cameraPosPitchCorrected.x(), cameraPosPitchCorrected.y() - 0.1f, cameraPosPitchCorrected.z(), 1.0f);
    Eigen::Vector4f outPos;
    Eigen::Vector4f outNormal;
    if (!Common::IsPlayerGrinding() && Common::fRaycast(rayStartPos, rayEndPos, outPos, outNormal, 21))
    {
        //printf("Collision occured from (%.3f, %.3f, %.3f) to (%.3f, %.3f, %.3f) at (%.3f, %.3f, %.3f)\n", DEBUG_VECTOR3(rayStartPos), DEBUG_VECTOR3(rayEndPos), DEBUG_VECTOR3(outPos));
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

HOOK(void, __fastcall, CustomCamera_CCameraUpdateParallel, 0x10FB770, Sonic::CCamera* This, void* Edx, const hh::fnd::SUpdateInfo& in_rUpdateInfo)
{
    originalCustomCamera_CCameraUpdateParallel(This, Edx, in_rUpdateInfo);

    Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
    bool skyGemCameraEnabled = NextGenSonic::m_skyGemEnabled && !NextGenSonic::m_skyGemLaunched && padState->IsDown(Sonic::EKeyState::eKeyState_RightTrigger);

    // Change factor to interpolate camera
    if (!m_skyGemCameraUpdated)
    {
        if (skyGemCameraEnabled)
        {
            m_skyGemCameraFactor = min(1.0f, m_skyGemCameraFactor + in_rUpdateInfo.DeltaTime * cCamera_skyGemFactorRate);
        }
        else
        {
            m_skyGemCameraFactor = max(-0.5f, m_skyGemCameraFactor - in_rUpdateInfo.DeltaTime * cCamera_skyGemFactorRate);
            if (m_skyGemCameraFactor <= -0.5f)
            {
                CustomCamera::m_skyGemCameraPitch = 0.0f;
            }
        }
    }

    auto& camera = This->m_MyCamera;
    auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
    if (m_skyGemCameraFactor > 0.0f)
    {
        bool updateCamera = skyGemCameraEnabled && !m_skyGemCameraUpdated;

        // Pitch
        if (!m_wasPaused)
        {
            CustomCamera::m_skyGemCameraPitch -= padState->LeftStickVertical * cCamera_skyGemRotateRate * in_rUpdateInfo.DeltaTime;
            Common::ClampFloat(CustomCamera::m_skyGemCameraPitch, cCamera_skyGemPitchMin, cCamera_skyGemPitchMax);
        }

        // Player vectors
        Hedgehog::Math::CVector playerRight = context->m_spMatrixNode->m_Transform.m_Rotation * Hedgehog::Math::CVector::UnitX();
        Hedgehog::Math::CVector playerUp = context->m_spMatrixNode->m_Transform.m_Rotation * Hedgehog::Math::CVector::UnitY();
        Hedgehog::Math::CVector playerForward = context->m_spMatrixNode->m_Transform.m_Rotation * Hedgehog::Math::CVector::UnitZ();
        Hedgehog::Math::CVector playerPosition = context->m_spMatrixNode->m_Transform.m_Position + playerUp * cCamera_skyGemtargetOffset;

        // Rotate player
        if (!m_wasPaused)
        {
            float yawAdd = -padState->LeftStickHorizontal * cCamera_skyGemRotateRate * in_rUpdateInfo.DeltaTime;
            context->m_HorizontalRotation = Eigen::AngleAxisf(yawAdd, playerUp) * context->m_HorizontalRotation;
        }
        
        // Camera vectors
        const Eigen::AngleAxisf rotPitch(CustomCamera::m_skyGemCameraPitch, playerRight);
        const Eigen::AngleAxisf rotYaw(180 * DEG_TO_RAD, playerUp);
        Hedgehog::Math::CVector cameraForward = rotPitch * rotYaw * playerForward;
        Hedgehog::Math::CQuaternion cameraRotation = updateCamera ? rotPitch * rotYaw * context->m_spMatrixNode->m_Transform.m_Rotation : m_skyGemCameraReleasedRotation;

        // Interpolate position
        Hedgehog::Math::CVector targetPosition = updateCamera ? playerPosition + cameraForward * cCamera_skyGemToPlayerDist : m_skyGemCameraReleasedPosition;
        camera.m_Position += (targetPosition - camera.m_Position) * m_skyGemCameraFactor;
        camera.m_Direction = cameraForward;

        // Interpolate rotation
        Hedgehog::Math::CQuaternion rotationSlerp = Hedgehog::Math::CQuaternion(camera.m_View.rotation().inverse()).slerp(m_skyGemCameraFactor, cameraRotation);
        camera.m_View = (Eigen::Translation3f(camera.m_Position) * rotationSlerp).inverse().matrix();
        camera.m_InputView = camera.m_View;

        if (skyGemCameraEnabled)
        {
            m_skyGemCameraReleasedPosition = targetPosition;
            m_skyGemCameraReleasedRotation = cameraRotation;
        }
    }

    m_skyGemCameraUpdated = true;
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

    // Override camera for Gems
    INSTALL_HOOK(CustomCamera_CCameraUpdateParallel);
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
    m_skyGemCameraUpdated = false;
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
