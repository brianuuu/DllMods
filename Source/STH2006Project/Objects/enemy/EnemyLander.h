/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Replaced by eRounder and eCommander
/*----------------------------------------------------------*/

#pragma once

class EnemyLander : public Sonic::CObjectBase
{
public:
	// Dummy data struct
	static uint32_t const c_size = 0x280;
	INSERT_PADDING(0x74);
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	INSERT_PADDING(0x104);

	// New data
	bool m_isCommander = false;

public:
	static void applyPatches();
};

BB_ASSERT_OFFSETOF(EnemyLander, m_isCommander, EnemyLander::c_size);
