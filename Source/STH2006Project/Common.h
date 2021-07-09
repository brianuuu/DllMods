#pragma once

void** const PLAYER_CONTEXT = (void**)0x1E5E2F0;
void** const pModernSonicContext = (void**)0x1E5E2F8;
void** const pClassicSonicContext = (void**)0x1E5E304;

struct MatrixNodeSingleElementNode
{
    INSERT_PADDING(0x60);
    Eigen::Matrix4f local;
    Eigen::Matrix4f world;
    INSERT_PADDING(0x60);
};

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

inline bool CheckPlayerNodeExist(const Hedgehog::Base::CSharedString& name)
{
    void* context = *(void**)0x1E5E2F0;
    if (context)
    {
        void* player = *(void**)((char*)context + 0x110);
        if (player)
        {
            boost::shared_ptr<MatrixNodeSingleElementNode> node;
            FUNCTION_PTR(void, __thiscall, GetNode, 0x700B70, void* This, boost::shared_ptr<MatrixNodeSingleElementNode> & node, const Hedgehog::Base::CSharedString & name);
            GetNode(*(void**)((char*)player + 0x234), node, name);
            return (node ? true : false);
        }
    }
}

inline bool CheckPlayerSuperForm()
{
    void* pSonicContext = nullptr;
    if (!pSonicContext) pSonicContext = *pModernSonicContext;
    if (!pSonicContext) pSonicContext = *pClassicSonicContext;
    if (pSonicContext)
    {
        uint32_t superSonicAddress = (uint32_t)(pSonicContext)+0x1A0;
        return (*(void**)superSonicAddress ? true : false);
    }

    return false;
}

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
