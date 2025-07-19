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
	bool CanLock() const;
	bool CanDamage() const;
	void CreateShield(uint32_t otherActor) const;
	void PlaySingleVO(std::string const& name, std::string const& id);

	void SetHidden(bool hidden);
	void FollowPlayer();

private:
	// Mephiles shadow manager
	struct ShadowManager
	{
		int m_numKilledUnit = 0;
		bool m_spawnAttack = false;

		void DoCommand(bool isAttack, int count, float radius);
		void Advance(float dt);
	} m_shadowManager;
};
