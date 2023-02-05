/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2023
//	Description: Recreate QTE JumpBoard from Unleashed using Generation's Trick panel
//  Usage: Hijack SizeType param of AdlibTrickJump
//         First digit: original SizeType, 0-1 button combo mode, 2-3 single button spam mode
//         Button Combo Mode: for each 2 digits, left digit: number of buttons, right digit: difficulty
//         Button Spam Mode: for each 4 digits, 1st digit: difficulty, 2nd digit: [0-5] ABXYLBRB, last 2 digits: spam amount
//         Example 1: 3241500
//              3-button lv.2, 4-button lv.1, 5-button lv.0, SizeType 0
//         Example 2: 591
//              5-button lv.9, SizeType 1
//         Example 3: 60252
//              Spam X lv.5 60 times (vs Perfect Dark Gaia), SizeType 0
//         Example 4: 51805493
//              Spam B lv.6 5 times, Spam LB lv.9 5 times, SizeType 1
/*----------------------------------------------------------*/

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

