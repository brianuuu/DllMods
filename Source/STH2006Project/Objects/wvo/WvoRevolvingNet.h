/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2026
///	Description: wvo_revolvingnet object from 06
/*----------------------------------------------------------*/

#pragma once
class WvoRevolvingNet : public Sonic::CObjectBase, public Sonic::CSetObjectListener
	, public Sonic::IAnimationContext, public Sonic::CAnimationStateMachine
{
public:
	BB_SET_OBJECT_MAKE("WvoRevolvingNet");
	static void registerObject();

private:
	struct Data
	{
		float m_LaunchSpeed = 20.0f;
		float m_OutOfControl = 0.5f;
	} m_Data;

private:
	static float const c_spinTime;

	bool m_isSpinning = false;
	float m_cooldown = 0.0f;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelGate;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeModelGate;
	boost::shared_ptr<hh::mot::CSingleElementEffectMotionAll> m_spEffectMotionAll;

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

	// from IAnimationContext
	Hedgehog::Animation::CAnimationPose* GetAnimationPose() override { return m_spAnimPose.get(); }
	Hedgehog::Math::CVector GetVelocityForAnimationSpeed() override { return hh::math::CVector::Ones(); }
	Hedgehog::Math::CVector GetVelocityForAnimationChange() override { return hh::math::CVector::Ones(); }
};

