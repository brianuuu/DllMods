#include "GenericObject.h"

BB_SET_OBJECT_MAKE_HOOK(GenericObject)
void GenericObject::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(GenericObject);
}

void GenericObject::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	uint32_t dummy = 0u;
	char const* name = "Name";
	m_Data.m_Name = Sonic::CParamTypeList::Create(&dummy, name);
	m_Data.m_Name->m_pMember->m_DefaultValue = 1;
	m_Data.m_Name->m_pMember->m_pFuncData->m_ValueMax = 1;
	if (m_Data.m_Name)
	{
		m_Data.m_Name->AddRef();
	}
	in_rEditParam.CreateParamBase(m_Data.m_Name, name);

	in_rEditParam.CreateParamFloat(&m_Data.m_CullingRange, "CullingRange");
	in_rEditParam.CreateParamBool(&m_Data.m_HasCollision, "HasCollision");
	in_rEditParam.CreateParamBool(&m_Data.m_HasDamage, "HasDamage");
}

bool GenericObject::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(m_Data.m_Name->m_pMember->m_DefaultValueName, 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, m_pMember->m_CastShadow);

	SetCullingRange(m_Data.m_CullingRange);

	return true;
}

bool GenericObject::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	if (m_Data.m_HasCollision || m_Data.m_HasDamage)
	{
		// Rigid body, ColID_BasicCamTerrain + TypeReactRing
		AddRigidBody(m_spRigidBody, m_Data.m_Name->m_pMember->m_DefaultValueName.c_str(), m_Data.m_Name->m_pMember->m_DefaultValueName.c_str(), *(int*)0x1E0AF18, m_spMatrixNodeTransform, in_spDatabase);

		if (!m_Data.m_HasCollision)
		{
			Common::ToggleRigidBodyCollision(m_spRigidBody.get(), false);
		}

		if (m_Data.m_HasDamage)
		{
			AddEventCollision("Damage", m_spRigidBody->GetShape(), *(int*)0x1E0AFD8, true, m_spMatrixNodeTransform); // ColID_PlayerEvent
		}
	}

	return true;
}

void GenericObject::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
}

bool GenericObject::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgHitEventCollision>())
		{
			if (m_Data.m_HasDamage)
			{
				SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
					(
						*(uint32_t*)0x1E0BE28, // DamageID_Heavy
						m_spMatrixNodeTransform->m_Transform.m_Position,
						hh::math::CVector::Zero()
					)
				);
			}
			return true;
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}
