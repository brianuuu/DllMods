/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Replaced by eLiner and eChaser
/*----------------------------------------------------------*/

#pragma once
class EnemyMotora : public Sonic::CObjectBase
{
public:
	static uint32_t const c_size = 0x260;
	INSERT_PADDING(0x18);
	uint32_t m_energyAmount = 1;
	INSERT_PADDING(0x144);

	// New data
	bool m_isChaser = false;

public:
	static void applyPatches();

private:
	static void __fastcall AddCallback(EnemyMotora* This, void*, const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase);
};

BB_ASSERT_OFFSETOF(EnemyMotora, m_energyAmount, 0x118);
BB_ASSERT_OFFSETOF(EnemyMotora, m_isChaser, EnemyMotora::c_size);
