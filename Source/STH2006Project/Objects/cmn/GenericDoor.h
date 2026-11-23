/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2026
///	Description: Generic door that triggers single animation
/*----------------------------------------------------------*/

#pragma once
class GenericDoor : public Sonic::CObjectBase, public Sonic::CSetObjectListener
	, public Sonic::IAnimationContext, public Sonic::CAnimationStateMachine
{
public:
	BB_SET_OBJECT_MAKE("GenericDoor");
	static void registerObject();

	virtual ~GenericDoor() override;

private:
	struct Data
	{
		float m_CullingRange = 50.0f;
		Sonic::CParamTypeList* m_Name = nullptr;
		int m_CueID = 0;
		bool m_DefaultTriggered = false;
	} m_Data;

private:
	bool m_triggered = false;
	bool m_useBank = false;
	SharedPtrTypeless m_sfx;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;

	// some door has effect animations
	boost::shared_ptr<hh::mot::CSingleElementEffectMotionAll> m_spEffectMotionAll;
	float m_effectTime = 0.0f;
	float m_effectEndTime = 0.0f;

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddParameterBank(const Hedgehog::Base::CRefPtr<Sonic::CParameterBank>& in_rParameterBank) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

	// from IAnimationContext
	Hedgehog::Animation::CAnimationPose* GetAnimationPose() override { return m_spAnimPose.get(); }
	Hedgehog::Math::CVector GetVelocityForAnimationSpeed() override { return hh::math::CVector::Ones(); }
	Hedgehog::Math::CVector GetVelocityForAnimationChange() override { return hh::math::CVector::Ones(); }

private:
	void Trigger(bool skip = false);
	void AddMatAnim(std::string const& name, bool skip = false);
};

