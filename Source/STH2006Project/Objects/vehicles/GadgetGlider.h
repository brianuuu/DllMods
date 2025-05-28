/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Gadget_Glider object from 06
/*----------------------------------------------------------*/

#pragma once
class GadgetGlider : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("GadgetGlider");
	static void registerObject();

	~GadgetGlider();

	enum class Direction
	{
		None,
		Up,
		Down,
		Left,
		Right
	};

private:
	struct Data
	{
		float m_Radius = 10.0f;
	} m_Data;

private:
	SharedPtrTypeless m_loopSfx;
	uint32_t m_burnerLID;
	uint32_t m_burnerRID;

	uint32_t m_playerID = 0u;
	Direction m_direction = Direction::None;
	float m_hp = 100.0f;
	float m_speed = 0.0f;
	hh::math::CVector2 m_steer = hh::math::CVector2::Zero();
	hh::math::CVector2 m_offset = hh::math::CVector2::Zero();
	hh::math::CVector m_splinePos = hh::math::CVector::Zero();
	hh::math::CQuaternion m_rotation = hh::math::CQuaternion::Identity();
	bool m_started = false;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBase;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBoosterL;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBoosterR;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeBoosterL;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeBoosterR;

	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spSonicControlNode;

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void DeathCallback(Sonic::CGameDocument* in_pGameDocument) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

private:
	bool IsValidPlayer() const;
	void BeginFlight();
	void TakeDamage(float amount);
	void Explode();
	Direction GetAnimationDirection(hh::math::CVector2 input) const;
	std::string GetAnimationName() const;
};

