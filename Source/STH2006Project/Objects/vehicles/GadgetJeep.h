/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Gadget_Bike object from 06
/*----------------------------------------------------------*/
#include "GadgetGunSimple.h"

#pragma once
class GadgetJeepBooster : public Sonic::CObjectBase
	, public Sonic::IAnimationContext, public Sonic::CAnimationStateMachine
{
private:
	boost::shared_ptr<hh::mr::CMatrixNode> m_spNodeParent;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;

	bool m_castShadow = true;
	float m_timer = 0.0f;

public:
	GadgetJeepBooster(boost::shared_ptr<hh::mr::CMatrixNode> parent, bool castShadow) : m_spNodeParent(parent), m_castShadow(castShadow) {}

	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;

	// from IAnimationContext
	Hedgehog::Animation::CAnimationPose* GetAnimationPose() override { return m_spAnimPose.get(); }
	Hedgehog::Math::CVector GetVelocityForAnimationSpeed() override { return hh::math::CVector::Ones(); }
	Hedgehog::Math::CVector GetVelocityForAnimationChange() override { return hh::math::CVector::Ones(); }

	bool IsBoosting() const;
	bool Boost();
	float GetBoostTime() const;
};

class GadgetJeep : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("GadgetJeep");
	static void registerObject();
	static void applyPatches();

	virtual ~GadgetJeep() override;

	enum class Direction
	{
		None,
		Left,
		Right,
		Back
	};

	enum class State
	{
		Idle,
		PlayerGetOn,
		Driving,
	};

private:
	struct Data
	{
		bool m_CanGetOff = true;
		bool m_DeadNoHP = false;
	} m_Data;

private:
	SharedPtrTypeless m_loopSfx;
	SharedPtrTypeless m_brakeSfx;
	uint32_t m_brokenID = 0;

	std::mutex m_mutex;
	uint32_t m_playerID = 0u;
	Direction m_direction = Direction::None;
	State m_state = State::Idle;

	hh::math::CVector2 m_input = hh::math::CVector2::Zero();
	hh::math::CQuaternion m_rotation = hh::math::CQuaternion::Identity();
	hh::math::CVector m_wheelFLPos = hh::math::CVector::Zero();
	hh::math::CVector m_wheelFRPos = hh::math::CVector::Zero();
	hh::math::CVector m_wheelBLPos = hh::math::CVector::Zero();
	hh::math::CVector m_wheelBRPos = hh::math::CVector::Zero();
	float m_wheelAngle = 0.0f;

	float m_hp = 100.0f;
	float m_speed = 0.0f;
	float m_upSpeed = 0.0f;
	float m_outOfControl = 0.0f;
	float m_doubleTapTime = 0.0f;
	bool m_started = false;
	bool m_brakeLights = false;
	bool m_isLanded = true;

	struct PlayerGetOnData
	{
		hh::math::CVector m_start = hh::math::CVector::Zero();
		float m_time = 0.0f;
	} m_playerGetOnData;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBase;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBodyWheelFL;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBodyWheelFR;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBodyWheelBL;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBodyWheelBR;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelGuardL;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModelGuardR;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModelWheelFL;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModelWheelFR;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModelWheelBL;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModelWheelBR;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeGuardL;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeGuardR;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeWheelFL;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeWheelFR;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeWheelBL;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeWheelBR;

	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeCockpit;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spSonicControlNode;

	boost::shared_ptr<hh::mot::CSingleElementEffectMotionAll> m_spEffectMotionAll;

	boost::shared_ptr<GadgetGunSimple> m_spGunL;
	boost::shared_ptr<GadgetGunSimple> m_spGunR;
	boost::shared_ptr<GadgetJeepBooster> m_spBooster;

	boost::shared_ptr<Sonic::CCharacterProxy> m_spProxy;

private:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void KillCallback() override;
	void GetObjectTriggerType(hh::vector<uint32_t>& in_rTriggerTypeList) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

private:
	bool IsValidPlayer() const;
	bool IsDriving() const;

	void BeginPlayerGetOn();
	void AdvancePlayerGetOn(float dt);
	void BeginPlayerGetOff(bool isAlive);

	void UnloadGun();
	void CleanUp();

	void BeginDriving();
	void AdvanceDriving(float dt);
	void AdvancePhysics(float dt);

	void ToggleBrakeLights(bool on);
	void TakeDamage(float amount);
	void Explode();

	Direction GetCurrentDirection(hh::math::CVector2 input) const;
	std::string GetAnimationName() const;
};

