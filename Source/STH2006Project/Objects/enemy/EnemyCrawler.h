/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Replaced by cCrawler and cGazer
/*----------------------------------------------------------*/

#pragma once
class EnemyCrawler : public Sonic::CObjectBase
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
	static void __fastcall AddCallback(EnemyCrawler* This, void*, void*);
};

BB_ASSERT_OFFSETOF(EnemyCrawler, m_energyAmount, 0x118);
BB_ASSERT_OFFSETOF(EnemyCrawler, m_isDark, EnemyCrawler::c_size);

// Dark togeball data used by Sonic::CEnemyShot
struct DarkTogeballExplosionData
{
	Hedgehog::Base::CSharedString m_effectName;
	uint32_t m_cueID = 5162007;
	float m_field08 = 1.0f;
	float m_field0C = 1.0f;
	float m_field10 = 0.0f;
	uint32_t m_damageType = 0;

	bool m_init = false;
};
static DarkTogeballExplosionData m_darkTogeballExplosionData;
