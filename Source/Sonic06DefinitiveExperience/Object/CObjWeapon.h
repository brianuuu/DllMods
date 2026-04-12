/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2026
///	Description: Weapon object used by Shadow
/*----------------------------------------------------------*/

#pragma once
enum WeaponType
{
	WT_EggPawnGun,

	WT_COUNT,

	WT_GunFirst = WT_EggPawnGun,
	WT_GunLast = WT_EggPawnGun,
};

struct WeaponData
{
	std::string m_weaponModelName;
	int m_ammo = 0;
	int const m_maxAmmo = 0;
	int const m_spriteIndex = 0;
	std::string const m_chargeEffectName;
	float const m_chargeTime = 0.0f;

	float const m_speed = 0.0f;
	float const m_gravity = 0.0f;
	float const m_radius = 0.1f;
	std::string const m_projectileModelName;
	std::string const m_projectileEffectName;
	std::string const m_muzzleEffectName;
	std::string const m_hitEffectName;

	uint32_t const m_chargeSfx = 0;
	uint32_t const m_shootSfx = 0;
	uint32_t const m_hitSfx = 0;

	void Reset()
	{
		m_ammo = m_maxAmmo;
	}
};

class CObjProjectile : public Sonic::CObjectBase
{
private:
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;

	float m_lifetime = 0.0f;
	hh::math::CVector m_position = hh::math::CVector::Zero();
	hh::math::CVector m_velocity = hh::math::CVector::Zero();

	// Fixed weapon data
	WeaponType m_type = WT_COUNT;
	WeaponData* m_pData = nullptr;

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
	static WeaponType m_type;
	static std::vector<WeaponData> m_weaponData;

	static void ResetWeaponData();
	static WeaponData& GetWeaponData(WeaponType type) { return m_weaponData[type]; }
	static void VerifySpriteIndex();
	static bool CanShoot();

	// switch weapon
	static void SetWeaponType(WeaponType type);
	static void NextGun();

public:
	CObjWeapon(boost::shared_ptr<hh::mr::CMatrixNode> parent);
	void Shoot();

private:
	boost::shared_ptr<hh::mr::CMatrixNode> m_spNodeParent;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeModel;

	WeaponData* m_pData = nullptr;

private:
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
};