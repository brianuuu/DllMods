/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Gadget_Hover object from 06
/*----------------------------------------------------------*/

#pragma once
class GadgetHoverSuspension : public Sonic::CObjectBase
	, public Sonic::IAnimationContext, public Sonic::CAnimationStateMachine
{
private:
	boost::shared_ptr<hh::mr::CMatrixNode> m_spNodeParent;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;

	bool m_castShadow = true;

public:
	GadgetHoverSuspension(boost::shared_ptr<hh::mr::CMatrixNode> parent, bool castShadow) : m_spNodeParent(parent), m_castShadow(castShadow) {}

	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;

	// from IAnimationContext
	Hedgehog::Animation::CAnimationPose* GetAnimationPose() override { return m_spAnimPose.get(); }
	Hedgehog::Math::CVector GetVelocityForAnimationSpeed() override { return hh::math::CVector::Ones(); }
	Hedgehog::Math::CVector GetVelocityForAnimationChange() override { return hh::math::CVector::Ones(); }
};

class GadgetHoverGun : public Sonic::CObjectBase
	, public Sonic::IAnimationContext, public Sonic::CAnimationStateMachine
{
private:
	boost::shared_ptr<hh::mr::CMatrixNode> m_spNodeParent;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;

	bool m_castShadow = true;
	bool m_started = false;
	bool m_loaded = true;
	uint32_t m_owner = 0;

public:
	GadgetHoverGun(boost::shared_ptr<hh::mr::CMatrixNode> parent, bool castShadow, uint32_t owner) : m_spNodeParent(parent), m_castShadow(castShadow), m_owner(owner) {}

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

class GadgetHover : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("GadgetHover");
	static void registerObject();
	static void applyPatches();

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
	} m_Data;

private:
	SharedPtrTypeless m_loopSfx;
	SharedPtrTypeless m_brakeSfx;
	uint32_t m_guardLID = 0;
	uint32_t m_guardRID = 0;

	std::mutex m_mutex;
	uint32_t m_playerID = 0u;
	Direction m_direction = Direction::None;
	State m_state = State::Idle;

	float m_hp = 100.0f;
	float m_speed = 0.0f;
	float m_guardAngle = 0.0f;
	float m_reloadTimer = 0.0f;
	float m_bulletTimer = 0.0f;
	uint8_t m_bullets = 100u;
	bool m_started = false;
	bool m_useGunL = true;
	bool m_brakeLights = false;
	bool m_guardLights = false;
	float m_guardLightTime = 0.0f;

	uint32_t m_landCollisionID = 0u;
	float m_jumpGuardTime = 0.0f;
	float m_jumpAccelTime = 0.0f;
	float m_doubleTapTime = 0.0f;
	float m_upSpeed = 0.0f;
	bool m_isLanded = true;

	struct PlayerGetOnData
	{
		hh::math::CVector m_start = hh::math::CVector::Zero();
		float m_time = 0.0f;
	} m_playerGetOnData;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBase;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelGuardL;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModelGuardR;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeGuardL;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeGuardR;

	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeCockpit;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spSonicControlNode;

	boost::shared_ptr<hh::mot::CSingleElementEffectMotionAll> m_spEffectMotionAll;
	boost::shared_ptr<hh::mot::CSingleElementEffectMotionAll> m_spEffectMotionGuardL;
	boost::shared_ptr<hh::mot::CSingleElementEffectMotionAll> m_spEffectMotionGuardR;

	boost::shared_ptr<GadgetHoverSuspension> m_spSuspension;
	boost::shared_ptr<GadgetHoverGun> m_spGunL;
	boost::shared_ptr<GadgetHoverGun> m_spGunR;

private:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void KillCallback() override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

private:
	bool IsValidPlayer() const;
	bool IsDriving() const;

	void BeginPlayerGetOn();
	void AdvancePlayerGetOn(float dt);
	void BeginPlayerGetOff(bool isAlive);

	void BeginDriving();
	void AdvanceDriving(float dt);

	void AdvanceGaurd(float dt);
	void AdvanceGuns(float dt);
	void AdvancePhysics(float dt);

	void ToggleBrakeLights(bool on);
	void TakeDamage(float amount);
	void Explode();

	Direction GetCurrentDirection(hh::math::CVector2 input) const;
	std::string GetAnimationName() const;
};
