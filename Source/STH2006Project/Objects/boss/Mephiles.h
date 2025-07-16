/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Mephiles boss from 06
/*----------------------------------------------------------*/

#pragma once
class Mephiles : public Sonic::CObjectBase, public Sonic::CSetObjectListener
	, public Sonic::IAnimationContext, public Sonic::CAnimationStateMachine
{
public:
	BB_SET_OBJECT_MAKE("Mephiles");
	static void registerObject();
	static void applyPatches();

	// for initial voice line
	static int m_encounterCount;

private:
	struct Data
	{
		int m_MaxHP = 10;
		float m_GroundHeight = 0.0f;
		hh::math::CVector m_SunDirection = hh::math::CVector::UnitZ();
	} m_Data;

	enum class State
	{
		Init,
		Hide,
		Ejected,
	};

private:
	State m_state = State::Init;
	State m_stateNext = State::Hide;
	hh::math::CQuaternion m_hideRotation = hh::math::CQuaternion::Identity();

	int m_HP = 10;
	bool m_playedInitVO = false;
	bool m_hasEjected = false;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;

	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeBody;

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

	// from IAnimationContext
	Hedgehog::Animation::CAnimationPose* GetAnimationPose() override { return m_spAnimPose.get(); }
	Hedgehog::Math::CVector GetVelocityForAnimationSpeed() override { return hh::math::CVector::Ones(); }
	Hedgehog::Math::CVector GetVelocityForAnimationChange() override { return hh::math::CVector::Ones(); }

private:
	void HandleStateChange();

	// State::Init
	void StateInitEnd();

	// State::Hide
	void StateHideBegin();
	void StateHideAdvance(float dt);
	void StateHideEnd();

	// Utils
	bool CanLock() const;
	bool CanDamage() const;
	void CreateShield(uint32_t otherActor) const;
};
