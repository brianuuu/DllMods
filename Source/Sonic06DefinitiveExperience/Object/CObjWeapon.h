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
	std::string const m_weaponModelName;
	std::string const m_projectileModelName;
	std::string const m_projectileEffectName;
	std::string const m_chargeEffectName;
	std::string const m_muzzleEffectName;
	std::string const m_hitEffectName;

	int m_ammo = 0;
	int const m_maxAmmo = 0;
	int const m_spriteIndex = 0;
	float const m_chargeTime = 0.0f;
	float const m_shootInterval = 0.5f;
	float const m_speed = 0.0f;
	float const m_gravity = 0.0f;
	float const m_radius = 0.1f;

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
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	void UpdateTransform(float dt = 0.0f);
};

class CObjWeapon : public Sonic::CObjectBase
{
public:
	static WeaponType m_type;
	static std::vector<WeaponData> m_weaponData;

	static void ResetWeaponData();
	static WeaponData& GetWeaponData(WeaponType type) { return m_weaponData[type]; }
	static void VerifySpriteIndex();
	static bool CanShoot();

	// switch weapon
	static void SetWeaponType(WeaponType type, bool updateHUD = true);
	static void NextGun();

public:
	CObjWeapon(boost::shared_ptr<hh::mr::CMatrixNode> parent);

	bool IsActive() const;
	bool CanRelease() const;
	void SetStateIdle();
	void SetStateAir();
	void UpdateBoneRotation();

private:
	boost::shared_ptr<hh::mr::CMatrixNode> m_spNodeParent;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeModel;
	boost::shared_ptr<hh::mr::CMatrixNodeSingleElementNode> m_spNodeMuzzle;

	WeaponData* m_pData = nullptr;

	float m_chargeTimer = 0.0f;
	uint32_t m_chargeID = 0;
	float m_shootTimer = 0.0f;

	enum class State 
	{
		Idle,
		AirCharge,
		AirFire,
	} m_state = State::Idle;

	mutable std::mutex m_mutex;

	struct RotateBoneData
	{
		uint16_t m_boneID = 0;
		hh::math::CQuaternion m_addRotation = hh::math::CQuaternion::Identity();
	} m_rotateBoneData;

private:
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;

	void Shoot();
};