#include "WvoRevolvingNet.h"

float const WvoRevolvingNet::c_spinTime = 20.0f / 60.0f;

BB_SET_OBJECT_MAKE_HOOK(WvoRevolvingNet)
void WvoRevolvingNet::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(WvoRevolvingNet);
}

void WvoRevolvingNet::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	in_rEditParam.CreateParamFloat(&m_Data.m_LaunchSpeed, "LaunchSpeed");
	in_rEditParam.CreateParamFloat(&m_Data.m_OutOfControl, "OutOfControl");
}

bool WvoRevolvingNet::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// net
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelData = wrapper.GetModelData("wvo_obj_net", 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelData);
	m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, m_pMember->m_CastShadow);

	// net2
	char const* net2 = "wvo_obj_net2";
	m_spNodeModelGate = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModelGate->m_Transform.SetPosition(hh::math::CVector(0.0f, 2.0, 0.0f));
	m_spNodeModelGate->NotifyChanged();
	m_spNodeModelGate->SetParent(m_spMatrixNodeTransform.get());
	boost::shared_ptr<hh::mr::CModelData> spModelGateData = wrapper.GetModelData(net2, 0);
	m_spModelGate = boost::make_shared<hh::mr::CSingleElement>(spModelGateData);
	m_spModelGate->BindMatrixNode(m_spNodeModelGate);
	Sonic::CGameObject::AddRenderable("Object", m_spModelGate, m_pMember->m_CastShadow);

	// animations
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, net2);
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	entries.push_back(hh::anim::SMotionInfo("WrongSide", net2, 1.0f, hh::anim::eMotionRepeatType_PlayOnce));
	m_spAnimPose->AddMotionInfo(&entries.front(), entries.size());
	m_spAnimPose->CreateAnimationCache();
	m_spModelGate->BindPose(m_spAnimPose);

	// states
	SetContext(this);
	AddAnimationState("WrongSide");

	// mat-anim
	m_spEffectMotionAll = boost::make_shared<hh::mot::CSingleElementEffectMotionAll>();
	m_spModelGate->BindEffect(m_spEffectMotionAll);

	FUNCTION_PTR(void, __thiscall, fpGetMaterialAnimData, 0x759720,
		hh::mot::CMotionDatabaseWrapper const& wrapper,
		boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData>&materialAnimData,
		hh::base::CSharedString const& name,
		uint32_t flag
	);

	FUNCTION_PTR(void, __thiscall, fpCreateMatAnim, 0x753910,
		Hedgehog::Motion::CSingleElementEffectMotionAll * This,
		boost::shared_ptr<hh::mr::CModelData> const& modelData,
		boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData> const& materialAnimData
	);

	hh::mot::CMotionDatabaseWrapper motWrapper(in_spDatabase.get());
	boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData> materialAnimData;
	fpGetMaterialAnimData(motWrapper, materialAnimData, "wvo_obj_net2_light", 0);
	fpCreateMatAnim(m_spEffectMotionAll.get(), spModelGateData, materialAnimData);

	SetCullingRange(20.0f);

	return true;
}

bool WvoRevolvingNet::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// Rigid body
	char const* rigidBodyName = "wvo_obj_net";
	AddRigidBody(m_spRigidBody, rigidBodyName, rigidBodyName, *(int*)0x1E0AFF0, m_spMatrixNodeTransform, in_spDatabase);

	// Gate (this is solid, no need rigid body)
	auto const attachNode = m_spModelGate->GetNode("Root");
	hk2010_2_0::hkpBoxShape* shapeEventTrigger = new hk2010_2_0::hkpBoxShape(4.0f, 4.0f, 0.2f);
	AddEventCollision("Attack", shapeEventTrigger, *(int*)0x1E0AFE8, true, attachNode);

	return true;
}

void WvoRevolvingNet::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll * This, float dt);
	fpUpdateMotionAll(m_spEffectMotionAll.get(), in_rUpdateInfo.DeltaTime);

	Update(in_rUpdateInfo);
	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);

	if (m_cooldown > 0.0f)
	{
		m_cooldown = max(0.0f, m_cooldown - in_rUpdateInfo.DeltaTime);

		if (m_isSpinning)
		{
			hh::math::CQuaternion const newRotation = Eigen::AngleAxisf(2.0f * PI_F * (m_cooldown / c_spinTime), -hh::math::CVector::UnitX()) * hh::math::CQuaternion::Identity();
			m_spNodeModelGate->m_Transform.SetRotation(newRotation);
			m_spNodeModelGate->NotifyChanged();

			if (m_cooldown == 0.0f)
			{
				m_isSpinning = false;
				Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Attack", true);
			}
		}
	}
}

bool WvoRevolvingNet::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (!flag)
	{
		return Sonic::CObjectBase::ProcessMessage(message, flag);
	}

	if (message.Is<Sonic::Message::MsgDamage>())
	{
		if (m_cooldown <= 0.0f)
		{
			auto& msg = static_cast<Sonic::Message::MsgDamage&>(message);
			if (msg.m_DamageType == Common::MakeDamageID(SonicCollision::TypeSonicSquatKick) || msg.m_DamageType == Common::MakeDamageID(SonicCollision::TypeSonicSliding))
			{
				hh::math::CVector const forward = m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ();
				hh::math::CVector const other = (msg.m_DamagePosition - m_spMatrixNodeTransform->m_Transform.m_Position).normalized();
				if (forward.dot(other) > 0)
				{
					SharedPtrTypeless sfx;
					Common::ObjectPlaySound(this, 200601002, sfx);

					m_isSpinning = true;
					m_cooldown = c_spinTime;
					Common::ObjectToggleEventCollision(m_spEventCollisionHolder.get(), "Attack", false);
				
					SendMessage(msg.m_SenderActorID, boost::make_shared<Sonic::Message::MsgApplyImpulse>(m_spMatrixNodeTransform->m_Transform.m_Position, -forward * m_Data.m_LaunchSpeed, ImpulseType::DashPanel, m_Data.m_OutOfControl));
				}
				else
				{
					m_cooldown = 1.0f;
					ChangeState("WrongSide");
				}
			}
		}
		return true;
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}
