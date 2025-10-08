/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Mephiles boss from 06
/*----------------------------------------------------------*/
#include "Objects/boss/MephilesShadow.h"

#pragma once
class Mephiles : public Sonic::CObjectBase, public Sonic::CSetObjectListener
	, public Sonic::IAnimationContext, public Sonic::CAnimationStateMachine
{
public:
	BB_SET_OBJECT_MAKE("Mephiles");
	static void registerObject();
	static void applyPatches();

	virtual ~Mephiles() override;

	// for initial voice line
	static int m_encounterCount;

private:
	struct Data
	{
		int m_MaxHP = 10;
		float m_GroundHeight = 0.0f;
		hh::math::CVector m_SunDirection = hh::math::CVector::UnitZ();
		Sonic::CParamTargetList* m_PositionList = nullptr;
		uint32_t m_CameraLock = 0;
		uint32_t m_CameraLockDive = 0;
		uint32_t m_CameraPan = 0;
		uint32_t m_CameraPanNoEase = 0;
		uint32_t m_FocusObject = 0;
	} m_Data;

	enum class State
	{
		Appear,
		Hide,
		Eject,
		Dive,
		Warp,
		Damage,
		HalfHP,
		Dead,

		AttackSphereS,
		AttackSphereL,
		AttackCharge,
	};

	// animation states
	static char const* HideLoop;
	static char const* HideCommand;
	static char const* Command;
	static char const* Dive; // hide
	static char const* Grin; // big sphere attack
	static char const* GrinLoop; // big sphere attack
	static char const* Shock; // damage
	static char const* Smile; // sphere attack
	static char const* Suffer; // eject
	static char const* Tired; // paralyze
	static char const* Wait; // idle

private:
	State m_state = State::Appear;
	State m_stateNext = State::Appear;
	int m_stateStage = 0;
	float m_stateTime = 0.0f;

	hh::math::CQuaternion m_hideRotation = hh::math::CQuaternion::Identity();

	int m_HP = 10;
	bool m_isHidden = false;
	bool m_playedInitVO = false;
	bool m_hasEjected = false;
	bool m_damagedThisFrame = false;
	bool m_playDamageVO = true;
	bool m_canDamage = false;
	bool m_enterHalfHP = false;
	bool m_enterLastHP = false;
	int m_nextDamageStrength = 0;
	float m_spawnBarrierTimer = 0.0f;
	uint32_t m_cameraActorID = 0;
	uint32_t m_handLID = 0;
	uint32_t m_handRID = 0;
	uint32_t m_footLID = 0;
	uint32_t m_footRID = 0;
	uint32_t m_barrierID = 0;

	std::set<std::string> m_playedVO;

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

	// State::Appear
	void StateAppearBegin();
	void StateAppearAdvance(float dt);

	// State::Hide
	void StateHideBegin();
	void StateHideAdvance(float dt);

	// State::Eject
	void StateEjectBegin();
	void StateEjectAdvance(float dt);

	// State::Dive
	float m_diveYaw = 0.0f;
	hh::math::CVector m_targetPos = hh::math::CVector::Zero();
	void StateDiveBegin();
	void StateDiveAdvance(float dt);

	// State::Warp
	int m_warpIndex = -1;
	hh::math::CVector m_warpPos = hh::math::CVector::Zero();
	void StateWarpBegin();
	void StateWarpAdvance(float dt);

	// State::Damage
	void StateDamageBegin();
	void StateDamageAdvance(float dt);

	// State::HalfHP
	bool m_playedHalfHPVO = false;
	void StateHalfHPBegin();
	void StateHalfHPAdvance(float dt);
	void StateHalfHPEnd();

	// State::Dead
	void StateDeadBegin();
	void StateDeadAdvance(float dt);

	// State::AttackSphereS
	void StateAttackSphereSBegin();
	void StateAttackSphereSAdvance(float dt);
	void StateAttackSphereSEnd();
	void FireSphereS();

	// State::AttackSphereL
	boost::shared_ptr<Sonic::CGameObject> m_darkSphereL;
	void StateAttackSphereLBegin();
	void StateAttackSphereLAdvance(float dt);
	void StateAttackSphereLEnd();
	void FireSphereL();

	// State::AttackCharge
	void StateAttackChargeBegin();
	void StateAttackChargeAdvance(float dt);
	void StateAttackChargeEnd();

	// Utils
	hh::math::CVector GetBodyPosition() const;
	bool CanLock() const;
	void CreateShield(hh::math::CVector const& otherPos) const;
	void ToggleBarrier(bool enabled);
	void PlaySingleVO(std::string const& name, std::string const& id);
	float GetHPRatio() const;

	void SetHidden(bool hidden);
	void FollowPlayer();
	void TurnTowardsPlayer(float dt);
	float GetAttackBeforeDelay() const;
	float GetAttackAfterDelay() const;

	int m_attackCount = 0;
	State ChooseAttackState();

	bool m_slowedTime = false;
	void ToggleSlowTime(bool enable);

	// Camera
	void SetFocusCameraPosition(hh::math::CVector const& pos);
	void HandleDisableCameraLock();

private:
	// Mephiles shadow
	std::map<uint32_t, boost::shared_ptr<MephilesShadow>> m_shadows;
	std::map<uint32_t, boost::shared_ptr<MephilesShadow>> m_shadowsAttached;
	int m_numKilledUnit = 0;
	float m_attachCountdown = 0.0f;
	SharedPtrTypeless m_attachSfx;

	// spawn param
	int m_maxSpawnCount = 0;
	int m_spawnCount = 0;
	float m_spawnTimer = 0.0f;
	float m_spawnRadius = 5.0f;
	float m_attackStartTime = 0.0f;
	float m_attackMaxDelay = 0.0f;
	std::set<uint32_t> m_spawnedActors;
	MephilesShadow::Type m_spawnType = MephilesShadow::Type::Encirclement;

	void SpawnEncirclement(int count, float radius);
	void SpawnCircleWait(int count, float radius, float attackStartTime, float attackMaxDelay, MephilesShadow::Type attackType);
	void AdvanceShadowSpawn(float dt);
	void AdvanceShadowExplode(float dt);
	void SetShadowAvoidMephiles(bool avoid);
};
