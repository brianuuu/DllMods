#include "ExpToSonic.h"

HOOK(void, __fastcall, ExpMsgGetHudPosition, 0x1096790, void* This, void* Edx, MsgGetHudPosition* message)
{
    if (message->m_type == 0)
    {
        Eigen::Vector3f sonicPosition;
        Eigen::Quaternionf sonicRotation;
        if (Common::GetPlayerTransform(sonicPosition, sonicRotation))
        {
            sonicPosition.y() += 0.5f; // half Sonic's height
            message->m_position = sonicPosition;
            return;
        }
    }

    originalExpMsgGetHudPosition(This, Edx, message);
}

void ExpToSonic::applyPatches()
{
    INSTALL_HOOK(ExpMsgGetHudPosition);
}
