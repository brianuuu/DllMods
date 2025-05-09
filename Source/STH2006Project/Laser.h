/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Recreating common_laser from 06
/*----------------------------------------------------------*/

#pragma once
class Laser : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("Laser");
	static void registerObject();

private:
	struct Data
	{
		int m_Number = 1u;
		float m_Width = 1.0f;
		float m_Interval = 0.0f;
		float m_OnTime = 0.0f;
		float m_OffTime = 0.0f;
		bool m_IsMoving = false;
		bool m_DefaultOn = true;
	} m_Data;

private:
	SharedPtrTypeless m_loopSfx;
	bool m_isOn = true;
	bool m_statusLoaded = false;
	float m_timer = 0.0f;

	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;

	struct NodeElement
	{
		boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
		boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNode;
	};
	std::vector<NodeElement> m_lasers;
	std::vector<NodeElement> m_beams;

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddParameterBank(Hedgehog::Base::CRefPtr<Sonic::CParameterBank>& in_pParameterBank) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

private:
	void LaserOff(bool bypassOffCheck = false);
	void LaserOn();
};

