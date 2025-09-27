/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Replaced by eBuster
/*----------------------------------------------------------*/

#pragma once
class EnemyELauncher : public Sonic::CObjectBase
{
public:
	static uint32_t const c_size = 0x2B0;
	INSERT_PADDING(0x18);
	uint32_t m_energyAmount = 1;
	INSERT_PADDING(0x194);

public:
	static void applyPatches();

private:
	static void __fastcall AddCallback(EnemyELauncher* This, void*, void*);
};

BB_ASSERT_OFFSETOF(EnemyELauncher, m_energyAmount, 0x118);
BB_ASSERT_SIZEOF(EnemyELauncher, EnemyELauncher::c_size);
