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
		Dead
	};

private:
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeBody;

	uint32_t m_owner = 0;
	Type m_type = Type::Encirclement;
	float m_speed = 0.0f;
	hh::math::CVector m_direction = hh::math::CVector::Zero();

	State m_state = State::Idle;
	State m_stateNext = State::Idle;
	float m_stateTime = 0.0f;
	float m_startY = 0.0f;

	// animation states
	static char const* Loop;
	static char const* BodyAttack;
	static char const* Catch;
	static char const* CatchMiss;
	static char const* Dead;

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
	bool m_targetLost = false;
	std::vector<uint32_t> m_escapeEnemies;
	void StateIdleAdvance(float dt);
	void StateIdleEnd();

	// State::Blown
	void StateBlownBegin();
	void StateBlownAdvance(float dt);
	void StateBlownEnd();

	// State::Shock
	SharedPtrTypeless m_shockSfx;
	uint32_t m_shockID = 0u;
	void StateShockBegin();
	void StateShockAdvance(float dt);
	void StateShockEnd();

	// Utils
	hh::math::CVector GetBodyPosition() const;
	bool CanDamagePlayer() const;

	void UpdatePosition(float dt);
	void FaceDirection(hh::math::CVector const& dir);
	hh::math::CVector GetPlayerDirection(float* distance = nullptr) const;

public:
	static float const c_DodgeSpeed;
	static float const c_ApproachSpeed;
	static float const c_EscapeSpeed;
	static float const c_MinEncirclementRadius;
	static float const c_MaxEncirclementRadius;
	static float const c_TargetLostDistance;
	static float const c_DeltaSpeed;
	static float const c_MinEncirclementHeight;
	static float const c_MaxEncirclementHeight;
};

