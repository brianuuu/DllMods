/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: shadows summoned by Mephiles
/*----------------------------------------------------------*/

#pragma once
class MephilesShadow : public Sonic::CObjectBase
	, public Sonic::IAnimationContext, public Sonic::CAnimationStateMachine
{
public:
	enum class Type
	{
		Encirclement,
		Spring,
		Attack
	};

	enum class State
	{
		Idle,
		Blown,
		Shock,
	};

private:
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeBody;

	uint32_t m_owner = 0;
	Type m_type = Type::Encirclement;
	hh::math::CVector m_velocity = hh::math::CVector::Zero();

	State m_state = State::Idle;
	State m_stateNext = State::Idle;
	float m_stateTime = 0.0f;

	// animation states
	static char const* Loop;

public:
	MephilesShadow(uint32_t owner, Type type, hh::math::CVector const& startPos);

	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;

	// from IAnimationContext
	Hedgehog::Animation::CAnimationPose* GetAnimationPose() override { return m_spAnimPose.get(); }
	Hedgehog::Math::CVector GetVelocityForAnimationSpeed() override { return hh::math::CVector::Ones(); }
	Hedgehog::Math::CVector GetVelocityForAnimationChange() override { return hh::math::CVector::Ones(); }

private:
	void HandleStateChange();

	// State::Idle
	void StateIdleAdvance(float dt);

	// State::Blown
	void StateBlownBegin();
	void StateBlownAdvance(float dt);

	// State::Shock
	SharedPtrTypeless m_shockSfx;
	uint32_t m_shockID = 0u;
	void StateShockBegin();
	void StateShockAdvance(float dt);
	void StateShockEnd();

	// Utils
	hh::math::CVector GetBodyPosition() const;
	bool CanDamagePlayer() const;

	void FacePlayer();
};

