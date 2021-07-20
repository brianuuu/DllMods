#pragma once

#define PI 3.141592

typedef void* CSonicContext;
CSonicContext* const PLAYER_CONTEXT = (CSonicContext*)0x1E5E2F0;
CSonicContext* const pModernSonicContext = (CSonicContext*)0x1E5E2F8;
CSonicContext* const pClassicSonicContext = (CSonicContext*)0x1E5E304;
CSonicContext* const pSuperSonicContext = (CSonicContext*)0x1E5E310;

uint32_t const CStringConstructor = 0x6621A0;
uint32_t const CStringDestructor = 0x661550;

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

struct MsgSetPosition
{
    INSERT_PADDING(0x10);
    Eigen::Vector3f position;
    INSERT_PADDING(0x4);
};

struct MsgSetRotation
{
    INSERT_PADDING(0x10);
    Eigen::Quaternionf rotation;
};

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != tinyxml2::XML_SUCCESS) { printf("XMLParse Error: %i\n", a_eResult); return a_eResult; }
#endif

using SharedPtrTypeless = boost::shared_ptr<void>;
typedef void* __fastcall CSonicSpeedContextPlaySound(void*, void*, SharedPtrTypeless&, uint32_t cueId, uint32_t);

namespace Common
{

inline bool IsStringEndsWith(std::string const& value, std::string const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline bool CheckPlayerNodeExist(const Hedgehog::Base::CSharedString& name)
{
    void* context = *PLAYER_CONTEXT;
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

    return false;
}

inline bool CheckCurrentStage(char const* stageID)
{
    char const* currentStageID = (char*)0x01E774D4;
    return strcmp(currentStageID, stageID) == 0;
}

inline bool CheckPlayerSuperForm()
{
    void* pSonicContext = *PLAYER_CONTEXT;
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

    const uint32_t result = *(uint32_t*)((uint32_t) * (void**)((uint32_t)*PLAYER_CONTEXT + 0x110) + 0xAC);
    if (!result) return false;

    float* pPos = (float*)(*(uint32_t*)(result + 0x10) + 0x70);
    position.x() = pPos[0];
    position.y() = pPos[1];
    position.z() = pPos[2];

    float* pRot = (float*)(*(uint32_t*)(result + 0x10) + 0x60);
    rotation.x() = pRot[0];
    rotation.y() = pRot[1];
    rotation.z() = pRot[2];
    rotation.w() = pRot[3];

    return true;
}

inline void SonicContextPlaySound(SharedPtrTypeless& soundHandle, uint32_t cueID, uint32_t flag)
{
    // Note: This doesn't work at result screen, use PlaySoundStatic instead
    void* pSonicContext = *PLAYER_CONTEXT;
    if (!pSonicContext) return;

    // Original code by Skyth: https://github.com/blueskythlikesclouds
    CSonicSpeedContextPlaySound* playSoundFunc = *(CSonicSpeedContextPlaySound**)(*(uint32_t*)pSonicContext + 0x74);
    playSoundFunc(pSonicContext, nullptr, soundHandle, cueID, flag);
}

inline void applyObjectPhysicsPosition(void* pObject, Eigen::Vector3f const& pos)
{
    FUNCTION_PTR(void*, __thiscall, processObjectMsgSetPosition, 0xEA2130, void* This, void* message);
    alignas(16) MsgSetPosition msgSetPosition;
    msgSetPosition.position = pos;
    processObjectMsgSetPosition(pObject, &msgSetPosition);
}

inline void applyObjectPhysicsRotation(void* pObject, Eigen::Quaternionf const& rot)
{
    FUNCTION_PTR(void*, __thiscall, processObjectMsgSetRotation, 0xEA20D0, void* This, void* message);
    alignas(16) MsgSetRotation msgSetRotation;
    msgSetRotation.rotation = rot;
    processObjectMsgSetRotation(pObject, &msgSetRotation);
}

} // namespace Common
