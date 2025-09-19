/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2024
//	Description: Shock enemies
/*----------------------------------------------------------*/

#pragma once

class CObjShock;
class EnemyShock
{
public:
	static void applyPatches();

	struct ShockData
	{
		float m_Timer;
		SharedPtrTypeless m_Sound;
		boost::shared_ptr<CObjShock> m_Effect;
	};

	static std::recursive_mutex m_mutex;
	static std::map<uint32_t, ShockData> m_shockData;
};

