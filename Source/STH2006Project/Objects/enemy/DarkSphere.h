/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Dark sphere projectile from 06
/*----------------------------------------------------------*/

#pragma once
class DarkSphere : public Sonic::CObjectBase
{
private:
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;

	// const members
	uint32_t const m_owner = 0;
	uint32_t const m_target = 0;
	float const m_speed = 0.0f;
	bool const m_isLarge = false;

	// members
	uint32_t m_effectID = 0;
	SharedPtrTypeless m_largeSfx;
	float m_lifetime = 0.0f;
	hh::math::CVector m_velocity = hh::math::CVector::Zero();

public:
	DarkSphere(uint32_t owner, uint32_t target, float m_speed, bool isLarge, hh::math::CVector const& startPos);

	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;

private:
	hh::math::CVector GetTargetPos() const;
	void SetInitialVelocity();
	void Explode();
};

