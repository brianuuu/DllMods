/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2025
//	Description: Enable enemy health bar
/*----------------------------------------------------------*/

#pragma once
class CObjHealth;
class EnemyHealth
{
public:
	static void applyPatches();

	struct HealthData
	{
		boost::shared_ptr<CObjHealth> m_spHealth;
	};

	static std::mutex m_mutex;
	static std::map<uint32_t, HealthData> m_healthData;
	static bool m_noDamage;

	static uint32_t GetHealthOffset(uint32_t pCEnemyBase);
	static uint32_t GetMaxHealth(uint32_t pCEnemyBase);
	static uint8_t GetHealth(uint32_t pCEnemyBase);
	static void SetHealth(uint32_t pCEnemyBase, uint8_t health);
};

