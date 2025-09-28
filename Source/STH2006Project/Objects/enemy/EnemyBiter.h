/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Replaced by cBiter and cStalker
/*----------------------------------------------------------*/

#pragma once
class EnemyBiter : public Sonic::CObjectBase
{
public:
	static uint32_t const c_size = 0x270;
	INSERT_PADDING(0x18);
	uint32_t m_energyAmount = 1;
	INSERT_PADDING(0x154);
	bool m_isDark = false;

public:
	static void applyPatches();

private:
	static void __fastcall AddCallback(EnemyBiter* This, void*, void*);
};

BB_ASSERT_OFFSETOF(EnemyBiter, m_energyAmount, 0x118);
BB_ASSERT_OFFSETOF(EnemyBiter, m_isDark, EnemyBiter::c_size);
