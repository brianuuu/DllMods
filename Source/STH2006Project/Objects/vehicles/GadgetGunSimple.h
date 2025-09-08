/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: gun object attach to vehicles, without fire animation
/*----------------------------------------------------------*/

#pragma once
class GadgetGunSimple : public Sonic::CObjectBase
	, public Sonic::IAnimationContext, public Sonic::CAnimationStateMachine
{
private:
	std::string m_modelName;
	boost::shared_ptr<hh::mr::CMatrixNode> m_spNodeParent;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;

	bool m_castShadow = true;
	bool m_started = false;
	float m_loadTimer = 0.0f;
	float m_reloadTime = 1.0f;
	uint32_t m_owner = 0;

public:
	GadgetGunSimple(std::string const& name, boost::shared_ptr<hh::mr::CMatrixNode> parent, bool castShadow, uint32_t owner, float reloadTime) : m_modelName(name), m_spNodeParent(parent), m_castShadow(castShadow), m_owner(owner), m_reloadTime(reloadTime) {}

	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;

	// from IAnimationContext
	Hedgehog::Animation::CAnimationPose* GetAnimationPose() override { return m_spAnimPose.get(); }
	Hedgehog::Math::CVector GetVelocityForAnimationSpeed() override { return hh::math::CVector::Ones(); }
	Hedgehog::Math::CVector GetVelocityForAnimationChange() override { return hh::math::CVector::Ones(); }

	// API
	bool IsLoaded() const;
	bool IsStarted() const;
	void UpdateTransform();

private:
	void FireMissile();
};

