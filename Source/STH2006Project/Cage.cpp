#include "Cage.h"

BB_SET_OBJECT_MAKE_HOOK(Cage)
void Cage::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(Cage);
}

void Cage::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	m_isOpen = false;
	m_moveDownAmount = 0.0f;
}

bool Cage::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData("cmn_cage_base", 0);
	m_spModelBase = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModelBase->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModelBase, m_pMember->m_CastShadow);

	// top
	m_spNodeModel = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModel->SetParent(m_spMatrixNodeTransform.get());
	boost::shared_ptr<hh::mr::CModelData> spModelData = wrapper.GetModelData("cmn_cage", 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelData);
	m_spModel->BindMatrixNode(m_spNodeModel);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, m_pMember->m_CastShadow);

	// uv-anim
	m_spEffectMotionAll = boost::make_shared<hh::mot::CSingleElementEffectMotionAll>();
	m_spModel->BindEffect(m_spEffectMotionAll);
	boost::shared_ptr<Hedgehog::Motion::CTexcoordAnimationData> texCoordAnimData;

	FUNCTION_PTR(void, __thiscall, fpGetTexCoordAnimData, 0x7597E0,
		hh::mot::CMotionDatabaseWrapper const& wrapper,
		boost::shared_ptr<Hedgehog::Motion::CTexcoordAnimationData>&texCoordAnimData,
		hh::base::CSharedString const& name,
		uint32_t flag
	);

	FUNCTION_PTR(void, __thiscall, fpCreateUVAnim, 0x7537E0,
		Hedgehog::Motion::CSingleElementEffectMotionAll * This,
		boost::shared_ptr<hh::mr::CModelData> const& modelData,
		boost::shared_ptr<Hedgehog::Motion::CTexcoordAnimationData> const& texCoordAnimData
	);

	hh::mot::CMotionDatabaseWrapper motWrapper(in_spDatabase.get());
	fpGetTexCoordAnimData(motWrapper, texCoordAnimData, "cmn_cage_wall-0000", 0);
	fpCreateUVAnim(m_spEffectMotionAll.get(), spModelData, texCoordAnimData);
	fpGetTexCoordAnimData(motWrapper, texCoordAnimData, "cmn_cage_wall-0001", 0);
	fpCreateUVAnim(m_spEffectMotionAll.get(), spModelData, texCoordAnimData);

	return true;
}

bool Cage::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// Rigid body
	char const* rigidBodyName = "cmn_cage_base";
	AddRigidBody(m_spRigidBody, rigidBodyName, rigidBodyName, *(int*)0x1E0AFF4, m_spMatrixNodeTransform, in_spDatabase);

	// Walls
	m_spWalls.resize(5);
	m_spNodeWalls.resize(5);
	for (int i = 0; i < 4; i++)
	{
		hh::math::CVector pos;
		switch (i)
		{
		case 0: pos = hh::math::CVector(-1.55, 1.66f, 0.0f); break;
		case 1: pos = hh::math::CVector(0.0f, 1.66f, -1.55); break;
		case 2: pos = hh::math::CVector(1.55, 1.66f, 0.0f); break;
		case 3: pos = hh::math::CVector(0.0f, 1.66f, 1.55); break;
		}

		m_spNodeWalls[i] = boost::make_shared<Sonic::CMatrixNodeTransform>();
		m_spNodeWalls[i]->m_Transform.SetPosition(pos);
		m_spNodeWalls[i]->NotifyChanged();
		m_spNodeWalls[i]->SetParent(m_spMatrixNodeTransform.get());
		hk2010_2_0::hkpBoxShape* pRigidBodyShape = i % 2 == 0 ? new hk2010_2_0::hkpBoxShape(0.4f, 3.0f, 3.5f) : new hk2010_2_0::hkpBoxShape(3.5f, 3.0f, 0.4f);
		AddRigidBody(m_spWalls[i], pRigidBodyShape, *(int*)0x1E0AFF0, m_spNodeWalls[i]);
	}

	m_spNodeWalls[4] = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeWalls[4]->m_Transform.SetPosition(hh::math::CVector(0.0f, 2.96f, 0.0f));
	m_spNodeWalls[4]->NotifyChanged();
	m_spNodeWalls[4]->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpBoxShape* pRigidBodyShape = new hk2010_2_0::hkpBoxShape(2.7f, 0.4f, 2.7f);
	AddRigidBody(m_spWalls[4], pRigidBodyShape, *(int*)0x1E0AFF0, m_spNodeWalls[4]);

	// Center event collision
	m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeEventCollision->m_Transform.SetPosition(hh::math::CVector(0.0f, 1.5f, 0.0f));
	m_spNodeEventCollision->NotifyChanged();
	m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());
	hk2010_2_0::hkpBoxShape* shapeEventTrigger = new hk2010_2_0::hkpBoxShape(2.3f, 2.3f, 2.3f);
	AddEventCollision("Trapped", shapeEventTrigger, *(int*)0x1E0AFD8, true, m_spNodeEventCollision);
	
	return true;
}

void Cage::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll* This, float dt);
	fpUpdateMotionAll(m_spEffectMotionAll.get(), in_rUpdateInfo.DeltaTime);

	float constexpr c_totalMoveAmount = 3.0f;
	float constexpr c_moveRate = c_totalMoveAmount / (35.0f / 60.0f);
	if (m_isOpen && m_moveDownAmount < c_totalMoveAmount)
	{
		m_moveDownAmount += in_rUpdateInfo.DeltaTime * c_moveRate;
		m_spNodeModel->m_Transform.SetPosition(hh::math::CVector(0.0f, -m_moveDownAmount, 0.0f));
		m_spNodeModel->NotifyChanged();
	}
}

void Cage::CGameObject2C
(
	void* pData
)
{
	FUNCTION_PTR(int, __stdcall, fpSetUpTrigger4, 0xEA2940, void* pData);
	fpSetUpTrigger4(pData);
}

bool Cage::ProcessMessage
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
			OpenCage();
		}
		return true;
	}

	if (message.Is<Sonic::Message::MsgHitEventCollision>())
	{
		OpenCage();
		return true;
	}

	if (message.Is<Sonic::Message::MsgDeactivate>())
	{
		if (m_isOpen)
		{
			auto& msg = static_cast<Sonic::Message::MsgDeactivate&>(message);
			msg.m_Flag = false;
		}
		return true;
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void Cage::OpenCage()
{
	if (m_isOpen) return;

	Common::fEventTrigger(this, 4);
	m_isOpen = true;

	// remove walls
	FUNCTION_PTR(void, __thiscall, fpDisableCollision, 0x10C0F40, Sonic::CRigidBody* This, Sonic::CPhysicsWorld* pWorld);
	for (int i = 0; i < 5; i++)
	{
		fpDisableCollision(m_spWalls[i].get(), m_spWalls[i]->m_pPhysicsWorld);
	}
}
