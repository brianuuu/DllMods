/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Gadget_Bike object from 06
/*----------------------------------------------------------*/

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
	std::mutex m_mutex;
	uint32_t m_playerID = 0u;
	Direction m_direction = Direction::None;
	State m_state = State::Idle;
	hh::math::CVector2 m_input = hh::math::CVector2::Zero();

	float m_hp = 100.0f;
	float m_speed = 0.0f;
	float m_reloadTimer = 0.0f;
	float m_bulletTimer = 0.0f;
	uint8_t m_bullets = 100u;
	bool m_started = false;
	bool m_useGunL = true;
	bool m_brakeLights = false;

	struct PlayerGetOnData
	{
		hh::math::CVector m_start = hh::math::CVector::Zero();
		float m_time = 0.0f;
	} m_playerGetOnData;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBase;

	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spSonicControlNode;

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

	void ToggleBrakeLights(bool on);
	void TakeDamage(float amount);
	void Explode();

	Direction GetCurrentDirection(hh::math::CVector2 input) const;
	std::string GetAnimationName() const;
};

