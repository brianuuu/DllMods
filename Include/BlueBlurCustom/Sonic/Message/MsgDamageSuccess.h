#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class MsgDamageSuccess : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        HH_FND_MSG_MAKE_TYPE(0x1681E6C);

        Hedgehog::Math::CVector m_Position;
        size_t m_Field20;
        size_t m_Field24; // value from MsgDamage + 0x14?
        float m_AwardBoostRatio; // used when m_AwardBoostType = 8, award boost by (MaxBoost * ratio)
        size_t m_AwardBoostType; // has to do with awarding boost
        bool m_Flag; // unknown?
        bool m_Field31; // always false?
        size_t m_DisableHitParticle;
        size_t m_DisableBounce;
        BB_INSERT_PADDING(0x4);

        // the same order as sub_4F9E90
        MsgDamageSuccess
        (
            Hedgehog::Math::CVector const& in_Position,
            bool in_Flag,
            size_t in_DisableHitParticle = 0,
            size_t in_DisableBounce = 0,
            size_t in_AwardBoostType = 0,
            float in_AwardBoostRatio = 0.0f,
            size_t in_Field20 = 0,
            size_t in_Field24 = 0
        ) 
            : m_Position(in_Position)
            , m_Flag(in_Flag)
            , m_Field20(in_Field20)
            , m_AwardBoostType(in_AwardBoostType)
            , m_Field24(in_Field24)
            , m_AwardBoostRatio(in_AwardBoostRatio)
            , m_Field31(0)
            , m_DisableHitParticle(in_DisableHitParticle)
            , m_DisableBounce(in_DisableBounce)
        {}
    };

    BB_ASSERT_OFFSETOF(MsgDamageSuccess, m_Position, 0x10);
    BB_ASSERT_OFFSETOF(MsgDamageSuccess, m_Field20, 0x20);
    BB_ASSERT_OFFSETOF(MsgDamageSuccess, m_Field24, 0x24);
    BB_ASSERT_OFFSETOF(MsgDamageSuccess, m_AwardBoostRatio, 0x28);
    BB_ASSERT_OFFSETOF(MsgDamageSuccess, m_AwardBoostType, 0x2C);
    BB_ASSERT_OFFSETOF(MsgDamageSuccess, m_Flag, 0x30);
    BB_ASSERT_OFFSETOF(MsgDamageSuccess, m_Field31, 0x31);
    BB_ASSERT_OFFSETOF(MsgDamageSuccess, m_DisableHitParticle, 0x34);
    BB_ASSERT_OFFSETOF(MsgDamageSuccess, m_DisableBounce, 0x38);
    BB_ASSERT_SIZEOF(MsgDamageSuccess, 0x40);
}