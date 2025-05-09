#include "Laser.h"

BB_SET_OBJECT_MAKE_HOOK(Laser)
void Laser::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(Laser);
}

void Laser::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	in_rEditParam.CreateParamInt(&m_Data.m_Number, "Number");
	in_rEditParam.CreateParamFloat(&m_Data.m_Width, "Width");
	in_rEditParam.CreateParamFloat(&m_Data.m_Interval, "Interval");
	in_rEditParam.CreateParamFloat(&m_Data.m_OnTime, "OnTime");
	in_rEditParam.CreateParamFloat(&m_Data.m_OffTime, "OffTime");
	in_rEditParam.CreateParamBool(&m_Data.m_IsMoving, "IsMoving");
	in_rEditParam.CreateParamBool(&m_Data.m_DefaultOn, "DefaultOn");
}

bool Laser::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument,
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// laser ends
	m_lasers.resize(m_Data.m_Number * 2);
	for (int i = 0; i < m_Data.m_Number * 2; i++)
	{
		bool const isLeft = (i % 2) == 0;
		m_lasers[i].m_spNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
		m_lasers[i].m_spNode->m_Transform.SetPosition(hh::math::CVector(m_Data.m_Width * (isLeft ? 0.5f : -0.5f), (i/2) * m_Data.m_Interval, 0.0f));
		m_lasers[i].m_spNode->m_Transform.SetRotation(hh::math::CQuaternion(Eigen::AngleAxisf(PI * (isLeft ? -0.5f : 0.5f), hh::math::CVector::UnitY())));
		m_lasers[i].m_spNode->NotifyChanged();
		m_lasers[i].m_spNode->SetParent(m_spMatrixNodeTransform.get());
		boost::shared_ptr<hh::mr::CModelData> spModelData = wrapper.GetModelData("cmn_laser", 0);
		m_lasers[i].m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelData);
		m_lasers[i].m_spModel->BindMatrixNode(m_lasers[i].m_spNode);
		Sonic::CGameObject::AddRenderable("Object", m_lasers[i].m_spModel, false);
	}

	// beams
	m_beams.resize(m_Data.m_Number);
	for (int i = 0; i < m_Data.m_Number; i++)
	{
		m_beams[i].m_spNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
		m_beams[i].m_spNode->m_Transform.SetPosition(hh::math::CVector(0.0f, i * m_Data.m_Interval, 0.0f));
		m_beams[i].m_spNode->m_Transform.m_Matrix.scale(hh::math::CVector(m_Data.m_Width, 1.0f, 1.0f));
		m_beams[i].m_spNode->NotifyChanged();
		m_beams[i].m_spNode->SetParent(m_spMatrixNodeTransform.get());
		boost::shared_ptr<hh::mr::CModelData> spModelData = wrapper.GetModelData("cmn_laserbeam", 0);
		m_beams[i].m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelData);
		m_beams[i].m_spModel->BindMatrixNode(m_beams[i].m_spNode);
		Sonic::CGameObject::AddRenderable("Object", m_beams[i].m_spModel, false);
	}

	// culling
	float const collisionHeight = (m_Data.m_Number - 1) * m_Data.m_Interval + 1.0f;
	Common::ObjectSetCullingRange(this, max(m_Data.m_Width * 0.5f, collisionHeight) + 10.0f);

	return true;
}

bool Laser::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	if (m_Data.m_Number == 1)
	{
		// Rigid body
		hk2010_2_0::hkpCapsuleShape* pRigidBodyShape = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(m_Data.m_Width * -0.5f, 0.0f, 0.0f), hh::math::CVector(m_Data.m_Width * 0.5f, 0.0f, 0.0f), 0.2f);
		AddRigidBody(m_spRigidBody, pRigidBodyShape, *(int*)0x1E0AFF0, m_spMatrixNodeTransform);

		// Damage
		hk2010_2_0::hkpCapsuleShape* pDamageShape = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(m_Data.m_Width * -0.5f, 0.0f, 0.0f), hh::math::CVector(m_Data.m_Width * 0.5f, 0.0f, 0.0f), 0.3f);
		AddEventCollision("Damage", pDamageShape, *(int*)0x1E0AFD8, true, m_spMatrixNodeTransform);
	}
	else
	{
		float const collisionHeight = (m_Data.m_Number - 1) * m_Data.m_Interval + 1.0f;
		m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
		m_spNodeEventCollision->m_Transform.SetPosition(hh::math::CVector(0.0f, collisionHeight * 0.5f - 0.5f, 0.0f));
		m_spNodeEventCollision->NotifyChanged();
		m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());

		// Rigid body
		hk2010_2_0::hkpBoxShape* pRigidBodyShape = new hk2010_2_0::hkpBoxShape(m_Data.m_Width, collisionHeight, 0.2f);
		AddRigidBody(m_spRigidBody, pRigidBodyShape, *(int*)0x1E0AFF0, m_spNodeEventCollision);

		// Damage
		hk2010_2_0::hkpBoxShape* shapeEventTrigger = new hk2010_2_0::hkpBoxShape(m_Data.m_Width, collisionHeight, 0.5f);
		AddEventCollision("Damage", shapeEventTrigger, *(int*)0x1E0AFD8, true, m_spNodeEventCollision);
	}

	return true;
}

void Laser::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	if ((!m_statusLoaded && !m_Data.m_DefaultOn) || !m_isOn)
	{
		LaserOff(true);
	}
	else
	{
		Common::ObjectPlaySound(this, m_Data.m_IsMoving ? 200600016 : 200600014, m_loopSfx);
	}

	if (m_Data.m_OffTime > 0.0f && m_Data.m_OnTime > 0.0f)
	{
		m_timer = m_isOn ? m_Data.m_OnTime : m_Data.m_OffTime;
	}
}

void Laser::AddParameterBank
(
	Hedgehog::Base::CRefPtr<Sonic::CParameterBank>& in_rParameterBank
)
{
	in_rParameterBank->AccessParameterBankBool("IsOn", &m_isOn);
	m_statusLoaded = true;
}

void Laser::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	if (m_Data.m_OffTime > 0.0f && m_Data.m_OnTime > 0.0f)
	{
		m_timer -= in_rUpdateInfo.DeltaTime;
		if (m_timer <= 0.0f)
		{
			if (m_isOn)
			{
				LaserOff();
				m_timer = m_Data.m_OffTime;
			}
			else
			{
				LaserOn();
				m_timer = m_Data.m_OnTime;
			}
		}
	}
}

bool Laser::ProcessMessage
(
	Hedgehog::Universe::Message& message,
	bool flag
)
{
	if (!flag)
	{
		return Sonic::CObjectBase::ProcessMessage(message, flag);
	}

	if (message.Is<Sonic::Message::MsgNotifyObjectEvent>())
	{
		auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
		if (msg.m_Event == 6)
		{
			LaserOn();
		}
		else if (msg.m_Event == 7)
		{
			LaserOff();
		}
		return true;
	}

	if (message.Is<Sonic::Message::MsgHitEventCollision>())
	{
		if (m_isOn)
		{
			FUNCTION_PTR(void, __thiscall, fpDamagePlayerFromObject, 0xEC8040, void* This, Hedgehog::Universe::Message & message);
			fpDamagePlayerFromObject(this, message);
		}
		return true;
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void Laser::LaserOff(bool bypassOffCheck)
{
	if (!bypassOffCheck && !m_isOn) return;
	m_isOn = false;

	m_loopSfx.reset();
	if (!bypassOffCheck)
	{
		SharedPtrTypeless sfx;
		Common::ObjectPlaySound(this, 200600015, sfx);
	}

	FUNCTION_PTR(void, __thiscall, fpDisableCollision, 0x10C0F40, Sonic::CRigidBody* This, Sonic::CPhysicsWorld* pWorld);
	fpDisableCollision(m_spRigidBody.get(), m_spRigidBody->m_pPhysicsWorld);
	Common::ObjectDisableEventCollision(m_spEventCollisionHolder.get());

	for (int i = 0; i < m_Data.m_Number; i++)
	{
		m_beams[i].m_spNode->m_Transform.SetPosition(hh::math::CVector(0.0f, i * m_Data.m_Interval + 1000.0f, 0.0f));
		m_beams[i].m_spNode->NotifyChanged();
	}
}

void Laser::LaserOn()
{
	if (m_isOn) return;
	m_isOn = true;

	Common::ObjectPlaySound(this, m_Data.m_IsMoving ? 200600016 : 200600014, m_loopSfx);

	FUNCTION_PTR(void, __thiscall, fpEnableCollision, 0x10C0F90, Sonic::CRigidBody* This, Sonic::CPhysicsWorld* pWorld);
	fpEnableCollision(m_spRigidBody.get(), m_spRigidBody->m_pPhysicsWorld);
	Common::ObjectEnableEventCollision(m_spEventCollisionHolder.get());

	for (int i = 0; i < m_Data.m_Number; i++)
	{
		m_beams[i].m_spNode->m_Transform.SetPosition(hh::math::CVector(0.0f, i * m_Data.m_Interval, 0.0f));
		m_beams[i].m_spNode->m_Transform.m_Matrix.scale(hh::math::CVector(m_Data.m_Width, 1.0f, 1.0f));
		m_beams[i].m_spNode->NotifyChanged();
	}
}
