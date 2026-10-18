#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

enum LockOnType
{
    Enemy = 0x1,
    ChaosSpear = 0x2,
};

namespace Sonic::Message
{
    class MsgGetLockOnType : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        static constexpr const char* ms_pType = "MsgGetLockOnType";

        bool IsOfType(const char* in_pType) const override
        {
            return strcmp(in_pType, ms_pType) == 0;
        }
           
        const char* GetType() const override
        {
            return ms_pType;
        }
		
		uint32_t* m_pType;

        MsgGetLockOnType(uint32_t* in_pType) : m_pType(in_pType) {}
        MsgGetLockOnType(uint32_t& in_rType) : m_pType(&in_rType) {}
    };

    BB_ASSERT_OFFSETOF(MsgGetLockOnType, m_pType, 0x10);
    BB_ASSERT_SIZEOF(MsgGetLockOnType, 0x14);
}