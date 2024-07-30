/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2024
///	Description: Custom object TrickJumper from Unleashed
/*----------------------------------------------------------*/

#pragma once

namespace Hedgehog::Motion
{
	class CTexcoordAnimationData;
}

class TrickJumper : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("TrickJumper");

public:
	struct Data
	{
		// SetData params
		float m_OutOfControl[2];
		float m_Pitch[2];
		float m_Speed[2];

		int m_TrickCount[3];
		float m_TrickTime[3];

		int m_Difficulty;
		int m_Score;
		bool m_IsSideView;
	};

private:
	Data m_Data;

	hh::math::CVector m_arcPeakPosition;
	float m_uiAppearTime;
	bool m_statChecked;

	boost::shared_ptr<hh::mr::CSingleElement> m_spSpawnedModel;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeRigidBody;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;
	boost::shared_ptr<hh::mot::CSingleElementEffectMotionAll> m_spEffectMotionAll;

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

public:
    static void registerObject();
	static void applyPatches();

	static float m_xAspectOffset;
	static float m_yAspectOffset;
	static void CalculateAspectOffsets();
};