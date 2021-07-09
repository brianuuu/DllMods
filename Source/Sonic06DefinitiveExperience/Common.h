#pragma once

void** const PLAYER_CONTEXT = (void**)0x1E5E2F0;
void** const pModernSonicContext = (void**)0x1E5E2F8;
void** const pClassicSonicContext = (void**)0x1E5E304;

struct MsgGetHudPosition
{
    INSERT_PADDING(0x10);
    Eigen::Vector3f m_position;
    INSERT_PADDING(0x4);
    float m_speed; // just a guess?
    uint32_t m_type;
};

namespace Common
{

inline bool GetPlayerTransform(Eigen::Vector3f& position, Eigen::Quaternionf& rotation)
{
    if (!*PLAYER_CONTEXT) return false;

    const uint32_t result = *(uint32_t*)((uint32_t) * (void**)((uint32_t)*PLAYER_CONTEXT + 0x110) + 172);
    if (!result) return false;

    float* pPos = (float*)(*(uint32_t*)(result + 16) + 112);
    position.x() = pPos[0];
    position.y() = pPos[1];
    position.z() = pPos[2];

    float* pRot = (float*)(*(uint32_t*)(result + 16) + 96);
    rotation.x() = pRot[0];
    rotation.y() = pRot[1];
    rotation.z() = pRot[2];
    rotation.w() = pRot[3];

    return true;
}

} // namespace Common
