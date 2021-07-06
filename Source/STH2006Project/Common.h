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

} // namespace Common
