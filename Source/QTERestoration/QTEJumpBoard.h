/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2023
///	Description: Recreate QTE JumpBoard from Unleashed using Generation's Trick panel
/// Usage: Hijack SizeType param of AdlibTrickJump
///        First digit: original SizeType, 0-1 button combo mode, 2-3 single button spam mode
///        Button Combo Mode: for each 2 digits, left digit: number of buttons, right digit: difficulty
///        Button Spam Mode: for each 4 digits, rightmost digit: difficulty, 2nd digit: [0-5] ABXYLBRB, leftmost 2 digits: spam amount
///        Example 1: 3241500
///             3-button lv.2, 4-button lv.1, 5-button lv.0, SizeType 0
///        Example 2: 591
///             5-button lv.9, SizeType 1
///        Example 3: 60252
///             Spam X lv.5 60 times (vs Perfect Dark Gaia), SizeType 0
///        Example 4: 51805493
///             Spam B lv.8 5 times, Spam LB lv.9 5 times, SizeType 1
/// Note: 
///     -If QTE fails, Sonic will continue with "ImpulseSpeedOnNormal"
///     -If QTE is successful, Sonic will goto where it would've landed when launched
///         with "ImpulseSpeedOnBoost" base on "OutOfControl"
///     -"OutOfControl" must be longer than maximum time to finish QTE otherwise Sonic will go backwards
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
    };

public:
    static void applyPatches();
    
    static Data m_data;
    static void GetQTEJumpBoardData(uint32_t ptr);
};

