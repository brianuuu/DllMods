/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Replaced by eBomber, eSweeper and eArmor
/*----------------------------------------------------------*/

#pragma once
class EnemyGanigani : public Sonic::CObjectBase
{
public:
	static uint32_t const c_size = 0x250;
	INSERT_PADDING(0x18);
	uint32_t m_energyAmount = 1;
	INSERT_PADDING(0x134);

	// New data
	bool m_isSweeper = false;
	bool m_isArmor = false;

public:
	static void applyPatches();

private:
	static void __fastcall AddCallback(EnemyGanigani* This, void*, void*);
};

BB_ASSERT_OFFSETOF(EnemyGanigani, m_energyAmount, 0x118);
BB_ASSERT_OFFSETOF(EnemyGanigani, m_isSweeper, EnemyGanigani::c_size);
BB_ASSERT_OFFSETOF(EnemyGanigani, m_isArmor, EnemyGanigani::c_size + 1);
