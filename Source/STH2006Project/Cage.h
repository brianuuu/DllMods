/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Cage object from 06
/*----------------------------------------------------------*/

#pragma once
class Cage : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("Cage");
	static void registerObject();

private:
	bool m_isOpen = false;
	float m_moveDownAmount = 0.0f;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBase;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeModel;

	std::vector<boost::shared_ptr<Sonic::CRigidBody>> m_spWalls;
	std::vector<boost::shared_ptr<Sonic::CMatrixNodeTransform>> m_spNodeWalls;

	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;
	boost::shared_ptr<hh::mot::CSingleElementEffectMotionAll> m_spEffectMotionAll;

public:
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	void CGameObject2C(void* This) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

private:
	void OpenCage();
};


