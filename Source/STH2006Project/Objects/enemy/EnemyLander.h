/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Replaced by eRounder and eCommander
/*----------------------------------------------------------*/

#pragma once

class EnemyLander : public Sonic::CObjectBase
{
public:
	static uint32_t const c_size = 0x280;
	INSERT_PADDING(0x18);
	uint32_t m_energyAmount = 1;
	INSERT_PADDING(0x164);

	// New data
	bool m_isCommander = false;

public:
	static void applyPatches();

private:
	static void __fastcall AddCallback(EnemyLander* This, void*, void*);
};

BB_ASSERT_OFFSETOF(EnemyLander, m_energyAmount, 0x118);
BB_ASSERT_OFFSETOF(EnemyLander, m_isCommander, EnemyLander::c_size);
