/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Recreating updownreel from 06
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
		float m_HeightStart = 0.0f;
		float m_HeightEnd = 0.0f;
		float m_Time = 0.0f;
		bool m_DefaultOn = true;
	} m_Data;

private:
	SharedPtrTypeless m_loopSfx;
	float m_currentHeight = 0.0f;
	float m_speed = 0.0f;
	uint32_t m_playerID = 0u;

	bool m_isOn = true;
	bool m_statusLoaded = false;

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
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddParameterBank(Hedgehog::Base::CRefPtr<Sonic::CParameterBank>& in_pParameterBank) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

private:
	float GetTargetSpaeed();
};

