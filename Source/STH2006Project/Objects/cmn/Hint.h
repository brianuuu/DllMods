/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Recreating common_hint and common_hint_collision from 06
/*----------------------------------------------------------*/

#pragma once
#include "Managers/MstManager.h"

class Hint : public Sonic::CObjectBase, public Sonic::CSetObjectListener
	, public Sonic::IAnimationContext, public Sonic::CAnimationStateMachine
{
public:
	BB_SET_OBJECT_MAKE("Hint");
	static void registerObject();
	static void applyPatches();

	virtual ~Hint() override;

	enum class Type
	{
		Default,
		CollisionOnly,
		EventOnly
	};

	enum class CharacterType
	{
		All,
		Sonic,
		Shadow,
		Silver,
		Tails,
		Amy,
		Knuckles,
		Omega,
		Rouge,
		Blaze
	};

private:
	struct Data
	{
		int m_Type = 0;
		int m_CharacterType = 0;
		hh::math::CVector m_CollisionSize;
		Sonic::CParamTypeList* m_HintFile = nullptr;
		Sonic::CParamTypeList* m_HintName = nullptr;
		Sonic::CParamTypeList* m_HintTimes = nullptr;
		bool m_UseVoiceTime = true;
		uint32_t m_NextHintID = 0u;
	} m_Data;

private:
	bool m_isPlaying = false;
	float m_timer = 0.0f;
	mst::TextEntry m_entry;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<hh::anim::CAnimationPose> m_spAnimPose;

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void GetObjectTriggerType(hh::vector<uint32_t>& in_rTriggerTypeList) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

	// from IAnimationContext
	Hedgehog::Animation::CAnimationPose* GetAnimationPose() override { return m_spAnimPose.get(); }
	Hedgehog::Math::CVector GetVelocityForAnimationSpeed() override { return hh::math::CVector::Ones(); }
	Hedgehog::Math::CVector GetVelocityForAnimationChange() override { return hh::math::CVector::Ones(); }

private:
	void PlayHint();
	bool IsCharacterMatch();
};

