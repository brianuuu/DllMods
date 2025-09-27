/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Replaced by cTaker and cTricker
/*----------------------------------------------------------*/

#pragma once
class EnemyTaker : public Sonic::CObjectBase
{
public:
	static uint32_t const c_size = 0x5A0;
	INSERT_PADDING(0x18);
	uint32_t m_energyAmount = 1;
	INSERT_PADDING(0x484);
	bool m_isDark = false;

public:
	static void applyPatches();

private:
	static void __fastcall AddCallback(EnemyTaker* This, void*, void*);
};

BB_ASSERT_OFFSETOF(EnemyTaker, m_energyAmount, 0x118);
BB_ASSERT_OFFSETOF(EnemyTaker, m_isDark, EnemyTaker::c_size);

