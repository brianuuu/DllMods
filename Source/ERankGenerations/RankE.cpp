#include "RankE.h"
#include <Hedgehog.h>

struct MatrixNodeSingleElementNode
{
    INSERT_PADDING(0x60);
    Eigen::Matrix4f local;
    Eigen::Matrix4f world;
    INSERT_PADDING(0x60);
};

HOOK(void, __fastcall, MsgChangeResultState, 0xE692C0, void* This, void* Edx, uint32_t a2)
{
    uint32_t const state = *(uint32_t*)(a2 + 16);
    if (state == 1)
    {
        // Default Generations E-rank animation
        WRITE_MEMORY(0x15E8DAD, uint8_t, 0x67); // sg_result05_loop
        WRITE_MEMORY(0x15E8DC1, uint8_t, 0x67); // sg_result05
        WRITE_MEMORY(0x15D5FF2, uint8_t, 0x67); // ssg_result05_loop
        WRITE_MEMORY(0x15D6006, uint8_t, 0x67); // ssg_result05

        void* context = *(void**)0x1E5E2F0;
        if (context)
        {
            void* player = *(void**)((char*)context + 0x110);
            if (player)
            {
                boost::shared_ptr<MatrixNodeSingleElementNode> node;
                FUNCTION_PTR(void, __thiscall, GetNode, 0x700B70, void* This, boost::shared_ptr<MatrixNodeSingleElementNode> & node, const Hedgehog::Base::CSharedString & name);
                GetNode(*(void**)((char*)player + 0x234), node, "SonicRoot");

                if (node)
                {
                    // Unleashed E-rank animation
                    WRITE_MEMORY(0x15E8DAD, uint8_t, 0x6E); // sn_result05_loop
                    WRITE_MEMORY(0x15E8DC1, uint8_t, 0x6E); // sn_result05
                    WRITE_MEMORY(0x15D5FF2, uint8_t, 0x6E); // ssn_result05_loop
                    WRITE_MEMORY(0x15D6006, uint8_t, 0x6E); // ssn_result05
                }
            }
        }
    }

    originalMsgChangeResultState(This, Edx, a2);
}

void RankE::applyPatches()
{
    INSTALL_HOOK(MsgChangeResultState);

	WRITE_MEMORY(0x15E8DB6, uint8_t, 0x35); // sn_result00_loop -> sn_result05_loop
	WRITE_MEMORY(0x15E8DCA, uint8_t, 0x35); // sn_result00 -> sn_result05
	WRITE_MEMORY(0x15D5FFB, uint8_t, 0x35); // ssn_result00_loop -> ssn_result05_loop
	WRITE_MEMORY(0x15D600F, uint8_t, 0x35); // ssn_result00 -> ssn_result05

	WRITE_MEMORY(0x15EFE9D, uint8_t, 0x45); // SonicRankS -> SonicRankE
}
