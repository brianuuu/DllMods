/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2026
///	Description: Weapon object used by Shadow
/*----------------------------------------------------------*/

#pragma once
enum WeaponType
{
	WT_EggPawnGun,
	WT_COUNT
};

class CObjProjectile : public Sonic::CObjectBase
{
private:
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;

	float m_lifetime = 0.0f;
	hh::math::CVector m_position = hh::math::CVector::Zero();
	hh::math::CVector m_velocity = hh::math::CVector::Zero();

	// Fixed weapon data
	WeaponType m_type = WT_EggPawnGun;
	float m_speed = 0.0f;
	float m_gravity = 0.0f;
	std::string m_modelName;
	std::string m_effectName;
	std::string m_hitEffectName;
	uint32_t m_shootSfx = 0;
	uint32_t m_hitSfx = 0;

public:
	CObjProjectile(WeaponType type, hh::mr::CTransform const& startTrans, hh::math::CVector const& targetPos = hh::math::CVector::Zero());

private:
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	void UpdateTransform(float dt = 0.0f);
};

class CObjWeapon : public Sonic::CGameObject3D
{
public:
	
	struct WeaponData
	{
		WeaponData
		(
			std::string modelName,
			int maxAmmo
		)
		: m_modelName(modelName)
		, m_maxAmmo(maxAmmo)
		{
			Reset();
		}

		void Reset()
		{
			m_ammo = m_maxAmmo;
		}

		std::string m_modelName;

		float const m_shotInterval = 1.0f;
		int const m_maxAmmo = 0;
		int m_ammo = 0;
	};

	static std::vector<WeaponData> m_weaponData;
	static void ResetWeaponData();
	static WeaponData& GetWeaponData(WeaponType type) { return m_weaponData[type]; }

public:
	CObjWeapon(boost::shared_ptr<hh::mr::CMatrixNode> parent, WeaponType type);
	void Shoot();

private:
	boost::shared_ptr<hh::mr::CMatrixNode> m_spNodeParent;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeModel;

	WeaponType const m_type = WT_EggPawnGun;
	WeaponData* m_pData = nullptr;
	float m_shotTimer = 0.0f;

private:
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void KillCallback() override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;

};