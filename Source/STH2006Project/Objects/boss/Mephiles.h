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
		Appear,
		Hide,
		Eject,
	};

	// animation states
	static char const* HideLoop;
	static char const* HideCommand;
	static char const* Suffer;
	static char const* Wait;

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

	std::set<std::string> m_playedVO;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeBody;

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
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
	void HandleStateChange();

	// State::Appear
	void StateAppearBegin();
	void StateAppearAdvance(float dt);

	// State::Hide
	void StateHideBegin();
	void StateHideAdvance(float dt);

	// State::Eject
	void StateEjectBegin();

	// Utils
	hh::math::CVector GetBodyPosition() const;
	bool CanLock() const;
	bool CanDamage() const;
	void CreateShield(hh::math::CVector const& otherPos) const;
	void PlaySingleVO(std::string const& name, std::string const& id);

	void SetHidden(bool hidden);
	void FollowPlayer();

private:
	// Mephiles shadow
	std::map<uint32_t, boost::shared_ptr<MephilesShadow>> m_shadows;
	int m_numKilledUnit = 0;

	// spawn param
	int m_maxSpawnCount = 0;
	int m_spawnCount = 0;
	float m_spawnTimer = 0.0f;
	float m_spawnRadius = 5.0f;
	MephilesShadow::Type m_spawnType = MephilesShadow::Type::Encirclement;

	// parameters
	float c_DodgeSpeed = 0.5f;
	float c_ApproachSpeed = 6.0f;
	float c_EscapeSpeed = 6.0f;
	float c_MinEncirclementRadius = 8.0f;
	float c_MaxEncirclementRadius = 16.0f;
	float c_TargetLostDistance = 20.0f;
	float const c_MinEncirclementHeight = 0.75f;
	float const c_MaxEncirclementHeight = 3.5f;

	void SpawnEncirclement(int count, float radius);
	void SpawnSpring(int count, float radius, float attackStartTime, float maxDelay);
	void AdvanceSpawnShadow(float dt);

	hh::math::CVector GetShadowSpawnPosition() const;
};
