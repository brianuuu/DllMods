/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Replaced by eFlyer and eBuster
/*----------------------------------------------------------*/

#pragma once
class EnemyBeetle : public Sonic::CObjectBase
{
public:
	static uint32_t const c_size = 0x3D0; // GunBeetle = 0x3D0, MonoBeetle = 0x390
	INSERT_PADDING(0x18);
	uint32_t m_energyAmount = 1;
	INSERT_PADDING(0x2B4);

public:
	static void applyPatches();
	bool isBuster() const;

private:
	static void __fastcall AddCallback(EnemyBeetle* This, void*, void*);
};

BB_ASSERT_OFFSETOF(EnemyBeetle, m_energyAmount, 0x118);
BB_ASSERT_SIZEOF(EnemyBeetle, EnemyBeetle::c_size);
