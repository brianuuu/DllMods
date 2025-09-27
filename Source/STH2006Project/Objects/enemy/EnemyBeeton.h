/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Replaced by eSearcher and eHunter
/*----------------------------------------------------------*/

#pragma once
class EnemyBeeton : public Sonic::CObjectBase
{
public:
	static uint32_t const c_size = 0x2A0;
	INSERT_PADDING(0x18);
	uint32_t m_energyAmount = 1;
	INSERT_PADDING(0x60);
	bool m_isSearcher = false;
	INSERT_PADDING(0x123);

public:
	static void applyPatches();

private:
	static void __fastcall AddCallback(EnemyBeeton* This, void*, void*);
};

BB_ASSERT_OFFSETOF(EnemyBeeton, m_energyAmount, 0x118);
BB_ASSERT_OFFSETOF(EnemyBeeton, m_isSearcher, 0x17C);
BB_ASSERT_SIZEOF(EnemyBeeton, EnemyBeeton::c_size);
