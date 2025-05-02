/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Recreating UpDownReel from 06
/*----------------------------------------------------------*/

#pragma once
class UpDownReel : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("UpDownReel");
	static void registerObject();
	static void applyPatches();

private:
	struct Data
	{
		float m_HeightStart;
		float m_HeightEnd;
		float m_Time;
	} m_Data;

private:
	SharedPtrTypeless m_loopSfx;
	float m_currentHeight;
	float m_speed;
	uint32_t m_playerID;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBase;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBeam;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeModelBeam;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelHandle;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeModelHandle;

	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeModelPlayer;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

private:
	float GetTargetSpaeed();
};

