/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Gadget_Bike object from 06
/*----------------------------------------------------------*/
#include "GadgetGun.h"

#pragma once
class GadgetBike : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("GadgetBike");
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
		bool m_HasGun = false;
	} m_Data;

private:
	uint32_t m_brokenID = 0;

	std::mutex m_mutex;
	uint32_t m_playerID = 0u;
	Direction m_direction = Direction::None;
	State m_state = State::Idle;

	hh::math::CVector2 m_input = hh::math::CVector2::Zero();
	hh::math::CQuaternion m_rotation = hh::math::CQuaternion::Identity();
	float m_wheelAngle = 0.0f;
	float m_tiltAngle = 0.0f;

	float m_hp = 100.0f;
	float m_speed = 0.0f;
	float m_reloadTimer = 0.0f;
	float m_bulletTimer = 0.0f;
	uint8_t m_bullets = 100u;
	bool m_started = false;
	bool m_useGunL = true;
	bool m_brakeLights = false;
	bool m_isLanded = true;

	struct PlayerGetOnData
	{
		hh::math::CVector m_start = hh::math::CVector::Zero();
		float m_time = 0.0f;
	} m_playerGetOnData;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBase;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelWheelF;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModelWheelB;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeWheelF;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeWheelB;

	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeCockpit;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spSonicControlNode;

	boost::shared_ptr<hh::mot::CSingleElementEffectMotionAll> m_spEffectMotionAll;

	boost::shared_ptr<GadgetGun> m_spGunL;
	boost::shared_ptr<GadgetGun> m_spGunR;

private:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
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

	void AdvanceGuns(float dt);
	void AdvancePhysics(float dt);

	void ToggleBrakeLights(bool on);
	void TakeDamage(float amount);
	void Explode();

	Direction GetCurrentDirection(hh::math::CVector2 input) const;
	std::string GetAnimationName() const;
};

