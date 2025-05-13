/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: common_switch object from 06
/*----------------------------------------------------------*/

#pragma once
class BallSwitch : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("BallSwitch");
	static void registerObject();

	virtual ~BallSwitch() override;

	enum class Type
	{
		SwitchOnOnly,
		SwitchOnWhileStanding,
		SwitchOffByTimer
	};

private:
	BB_INSERT_PADDING(0xC); // 0x104, set 0x10F in sub_1002580

	struct Data 
	{
		Sonic::CParamTargetList* m_TargetListOFF = nullptr;
		Sonic::CParamTargetList* m_TargetListON = nullptr;
		int m_EventOFF = 0;
		int m_EventON = 0;
		float m_TimerOFF = 0.0f;
		float m_TimerON = 0.0f;
		bool m_OffBeep = true;
		float m_OffTimer = 0.0f;
		int m_Type;
	} m_Data;

private:
	SharedPtrTypeless m_onOffSfx;
	SharedPtrTypeless m_beepingSfx;
	float m_eventTimer = 0.0f;
	float m_offTimer = 0.0f;
	bool m_isOn = false;
	bool m_isHit = false;

	hh::math::CVector m_velocity = hh::math::CVector::Zero();
	float m_angle = 0.0f;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBase;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelBall;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeModelBall;

	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	void GetObjectTriggerType(hh::vector<uint32_t>& in_rTriggerTypeList) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

private:
	void SwitchOn();
	void SwitchOff();

	void SendEventOn();
	void SendEventOff();
};

