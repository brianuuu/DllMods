#pragma once

class QTEJumpBoard
{
public:
    struct Data
    {
        Data() : m_init(false) {}

        bool m_init;

        // tempory data, wait for MsgApplyImpulse
        uint32_t m_sizeType;
        float m_outOfControl;
        float m_impulseSpeedOnNormal;
        float m_impulseSpeedOnBoost;

        // data from MsgApplyImpulse
        Eigen::Vector3f m_position;
        Eigen::Vector3f m_velocity;
    };

public:
    static void applyPatches();
    
    static Data m_data;
    static void GetQTEJumpBoardData(uint32_t ptr);
};

