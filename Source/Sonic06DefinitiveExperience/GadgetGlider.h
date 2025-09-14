/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Gadget_Glider object from 06
/*----------------------------------------------------------*/
#include "GadgetGunSimple.h"

#pragma once
class GadgetGlider : public Sonic::CObjectBase
	, public Sonic::IAnimationContext, public Sonic::CAnimationStateMachine
{
public:
	GadgetGlider(hh::mr::CTransform const& startTrans, float speed, bool spawnGounded);
	virtual ~GadgetGlider() override;

	enum class Direction
	{
		None,
		Left,
		Right
	};

	enum class State
	{
		Idle,
		PlayerGetOn,
		Flight,
	};

private:
	SharedPtrTypeless m_loopSfx;
	uint32_t m_burnerLID = 0;
	uint32_t m_burnerRID = 0;
	uint32_t m_brokenID = 0;

	std::mutex m_mutex;
	uint32_t m_playerID = 0u;
	Direction m_direction = Direction::None;
	State m_state = State::Idle;

	float m_collisionEnableTimer = 0.0f;
	float m_hp = 100.0f;
	float m_speed = 0.0f;
	float m_pitch = 0.0f;
	bool m_spawnGrounded = true;

	hh::math::CVector2 m_steer = hh::math::CVector2::Zero();

	struct PlayerGetOnData
	{
		hh::math::CVector m_start = hh::math::CVector::Zero();
		float m_time = 0.0f;
	} m_playerGetOnData;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBase;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBodyMove;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBodyCockpit;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeModel;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBoosterL;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBoosterR;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeBoosterL;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeBoosterR;

	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeCockpit;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spSonicControlNode;

	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeExplodeL;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeExplodeR;

	boost::shared_ptr<GadgetGunSimple> m_spGunL;
	boost::shared_ptr<GadgetGunSimple> m_spGunR;

public:
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void KillCallback() override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

	// from IAnimationContext
	Hedgehog::Animation::CAnimationPose* GetAnimationPose() override { return m_spAnimPose.get(); }
	Hedgehog::Math::CVector GetVelocityForAnimationSpeed() override { return hh::math::CVector::Ones(); }
	Hedgehog::Math::CVector GetVelocityForAnimationChange() override { return hh::math::CVector::Ones(); }

private:
	bool IsFlight() const;

	void BeginPlayerGetOn();
	void AdvancePlayerGetOn(float dt);
	void BeginPlayerGetOff();

	void BeginFlight();
	void AdvanceFlight(float dt);

	void TakeDamage(float amount);
	void Explode();

	Direction GetAnimationDirection(hh::math::CVector2 input) const;
	std::string GetAnimationName() const;
};
