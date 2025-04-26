/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Switch object from 06
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
	struct Data 
	{
		Sonic::CParamTargetList* m_TargetListOFF;
		Sonic::CParamTargetList* m_TargetListON;
		int m_EventOFF;
		int m_EventON;
		float m_TimerOFF;
		float m_TimerON;
		bool m_OffBeep;
		float m_OffTimer;
		int m_Type;
	} m_Data;

private:
	SharedPtrTypeless m_onOffSfx;
	SharedPtrTypeless m_beepingSfx;
	float m_eventTimer;
	float m_offTimer;
	bool m_isOn;
	bool m_isHit;

	hh::math::CVector m_velocity;
	float m_angle;

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
	void CGameObject2C(void* This) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

private:
	void SwitchOn();
	void SwitchOff();

	void SendEventOn();
	void SendEventOff();
};

