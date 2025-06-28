/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Gadget_Hover object from 06
/*----------------------------------------------------------*/

#pragma once
class GadgetGun : public Sonic::CObjectBase
	, public Sonic::IAnimationContext, public Sonic::CAnimationStateMachine
{
private:
	boost::shared_ptr<hh::mr::CMatrixNode> m_spNodeParent;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;

	bool m_castShadow = true;
	bool m_started = false;
	bool m_loaded = true;
	bool m_sfxPlayed = true;
	uint32_t m_owner = 0;
	std::string m_name;

public:
	GadgetGun(std::string const& name, boost::shared_ptr<hh::mr::CMatrixNode> parent, bool castShadow, uint32_t owner) : m_name(name), m_spNodeParent(parent), m_castShadow(castShadow), m_owner(owner) {}

	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;

	// from IAnimationContext
	Hedgehog::Animation::CAnimationPose* GetAnimationPose() override { return m_spAnimPose.get(); }
	Hedgehog::Math::CVector GetVelocityForAnimationSpeed() override { return hh::math::CVector::Ones(); }
	Hedgehog::Math::CVector GetVelocityForAnimationChange() override { return hh::math::CVector::Ones(); }

	// API
	bool IsReady() const;
	bool CanUnload() const;
	void UpdateTransform();
	void FireBullet();
};

