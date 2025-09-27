/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Replaced by eStinger and eLancer
/*----------------------------------------------------------*/

#pragma once
class EnemyEggRobo : public Sonic::CObjectBase
{
public:
	static uint32_t const c_size = 0x4C0;
	INSERT_PADDING(0x18);
	uint32_t m_energyAmount = 1;
	INSERT_PADDING(0x84);
	bool m_isStinger = false;
	INSERT_PADDING(0x31F);

public:
	static void applyPatches();

private:
	static void __fastcall AddCallback(EnemyEggRobo* This, void*, void*);
};

BB_ASSERT_OFFSETOF(EnemyEggRobo, m_energyAmount, 0x118);
BB_ASSERT_OFFSETOF(EnemyEggRobo, m_isStinger, 0x1A0);
BB_ASSERT_SIZEOF(EnemyEggRobo, EnemyEggRobo::c_size);
