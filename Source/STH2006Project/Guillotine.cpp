#include "Guillotine.h"

BB_SET_OBJECT_MAKE_HOOK(Guillotine)
void Guillotine::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(Guillotine);
}

void Guillotine::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	m_Data.m_ShouldSpin = false;
	in_rEditParam.CreateParamBool(&m_Data.m_ShouldSpin, "ShouldSpin");
}

bool Guillotine::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelData = wrapper.GetModelData("cmn_guillotine", 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelData);
	m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, m_pMember->m_CastShadow);

	return true;
}

bool Guillotine::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// Rigid body
	hk2010_2_0::hkpCapsuleShape* pRigidBodyShape = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(-2.84f, 0.0f, 0.0f), hh::math::CVector(2.84f, 0.0f, 0.0f), 0.21f);
	AddRigidBody(m_spRigidBody, pRigidBodyShape, *(int*)0x1E0AFF0, m_spMatrixNodeTransform);

	// Damage
	hk2010_2_0::hkpCapsuleShape* pDamageShape = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(-2.4f, 0.0f, 0.0f), hh::math::CVector(2.4f, 0.0f, 0.0f), 0.6f);
	AddEventCollision("Damage", pDamageShape, *(int*)0x1E0AFD8, true, m_spMatrixNodeTransform);

	return true;
}

void Guillotine::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	float constexpr c_spinRate = 2.0f * PI * 2.0f;
	if (m_Data.m_ShouldSpin)
	{
		hh::math::CVector const rightAxis = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitX();
		hh::math::CQuaternion const newRotation = Eigen::AngleAxisf(c_spinRate * in_rUpdateInfo.DeltaTime, rightAxis) * m_spMatrixNodeTransform->m_Transform.m_Rotation;
		m_spMatrixNodeTransform->m_Transform.SetRotation(newRotation);
		m_spMatrixNodeTransform->NotifyChanged();
	}
}

bool Guillotine::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (!flag)
	{
		return Sonic::CObjectBase::ProcessMessage(message, flag);
	}

	if (message.Is<Sonic::Message::MsgLeaveEventCollision>())
	{
		FUNCTION_PTR(void, __thiscall, fpDamagePlayerFromObject, 0xEC8040, void* This, Hedgehog::Universe::Message& message);
		fpDamagePlayerFromObject(this, message);
		return true;
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}
