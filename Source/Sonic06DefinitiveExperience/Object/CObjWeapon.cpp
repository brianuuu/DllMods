#include "CObjWeapon.h"
#include "Character/NextGenShadow.h"
#include "Utils/AnimationSetPatcher.h"

WeaponType CObjWeapon::m_type = WT_EggPawnGun; // TODO: default WT_COUNT
std::vector<CObjWeapon::WeaponData> CObjWeapon::m_weaponData =
{
	// EggPawnGun
	{ "weapon_eggpawngun", 1, 50 },
};

CObjProjectile::CObjProjectile
(
	WeaponType type, 
	hh::mr::CTransform const& startTrans, 
	hh::math::CVector const& targetPos
)
	: m_type(type)
	, m_position(startTrans.m_Position)
{
	switch (m_type)
	{
	case WT_EggPawnGun:
	{
		m_speed = 20.0f;
		m_radius = 0.125f;
		m_effectName = "ef_eggpanwgun_bullet";
		m_muzzleEffectName = "ef_eggpanwgun_muzzle";
		m_hitEffectName = "ef_projectile_impact";
		break;
	}
	}

	// initial velocity
	if (targetPos.isZero())
	{
		m_velocity = startTrans.m_Rotation * hh::math::CVector::UnitZ() * m_speed;
		if (m_gravity > 0.0f)
		{
			// projectiles with gravity only set initial rotation
			m_spMatrixNodeTransform->m_Transform.SetRotation(startTrans.m_Rotation);
		}
	}
	else
	{
		hh::math::CVector const dir = (targetPos - m_position).normalized();
		hh::math::CVector dirXZ = dir; dirXZ.y() = 0.0f;

		if (m_gravity == 0.0f)
		{
			m_velocity = dir * m_speed;
		}
		else
		{
			// TODO: calculate initial speed to hit target
			hh::math::CQuaternion rotation = hh::math::CQuaternion::FromTwoVectors(Hedgehog::Math::CVector::UnitZ(), dirXZ.head<3>()) * startTrans.m_Rotation;
			m_velocity = rotation * hh::math::CVector::UnitZ() * m_speed;

			// projectiles with gravity only set initial rotation
			m_spMatrixNodeTransform->m_Transform.SetRotation(rotation);
		}
	}

	UpdateTransform();
}

void CObjProjectile::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder,
	Sonic::CGameDocument* in_pGameDocument,
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	if (m_shootSfx)
	{
		SharedPtrTypeless sfx;
		Common::ObjectPlaySound(this, m_shootSfx, sfx);
	}
}

bool CObjProjectile::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// model
	if (!m_modelName.empty())
	{
		hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
		boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(m_modelName.c_str(), 0);
		m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
		m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
		Sonic::CGameObject::AddRenderable("Object", m_spModel, true);
	}

	// effect
	if (!m_effectName.empty())
	{
		m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spMatrixNodeTransform, m_effectName.c_str(), 1.0f);
	}

	// muzzle
	if (!m_muzzleEffectName.empty())
	{
		auto effectNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
		effectNode->SetParent(Sonic::CApplicationDocument::GetInstance()->m_pMember->m_spMatrixNodeRoot.get());
		effectNode->m_Transform.SetPosition(m_spMatrixNodeTransform->m_Transform.m_Position);
		effectNode->NotifyChanged();
		m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, effectNode, m_muzzleEffectName.c_str(), 1.0f);
	}

	return true;
}

bool CObjProjectile::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// damage to object
	uint32_t const typeEnemy = *(uint32_t*)0x1E5E7E8;
	uint32_t const typeBreakable = *(uint32_t*)0x1E5E77C;
	uint32_t const typeTerrain = *(uint32_t*)0x1E5E754;
	uint64_t const bitfield = (1llu << typeEnemy) | (1llu << typeBreakable) | (1llu << typeTerrain);
	hk2010_2_0::hkpSphereShape* bodyEventTrigger = new hk2010_2_0::hkpSphereShape(m_radius);
	AddEventCollision("Attack", bodyEventTrigger, Common::MakeCollisionID(0, bitfield), true, m_spMatrixNodeTransform);

	return true;
}

bool CObjProjectile::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgRestartStage>() || message.Is<Sonic::Message::MsgStageClear>())
		{
			Kill();
			return true;
		}

		if (message.Is<Sonic::Message::MsgHitEventCollision>())
		{
			SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
				(
					*(uint32_t*)0x1E0BE28, // DamageID_SonicHeavy
					m_spMatrixNodeTransform->m_Transform.m_Position,
					m_velocity
				)
			);

			if (m_hitSfx)
			{
				SharedPtrTypeless sfx;
				Common::ObjectPlaySound(this, m_hitSfx, sfx);
			}

			if (!m_hitEffectName.empty())
			{
				m_pGlitterPlayer->PlayOneshot(m_spMatrixNodeTransform, m_hitEffectName.c_str(), 1.0f, 1);
			}

			Kill();
			return true;
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void CObjProjectile::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	UpdateTransform(in_rUpdateInfo.DeltaTime);
}

void CObjProjectile::UpdateTransform(float dt)
{
	m_velocity.y() -= m_gravity * dt;
	m_position += m_velocity * dt;

	if (m_gravity == 0.0f)
	{
		// Rotate spear to correct rotation
		Hedgehog::Math::CVector const dir = m_velocity.normalized();
		Hedgehog::Math::CVector dirXZ = dir; dirXZ.y() = 0.0f;
		Hedgehog::Math::CQuaternion rotYaw = Hedgehog::Math::CQuaternion::FromTwoVectors(Hedgehog::Math::CVector::UnitZ(), dirXZ.head<3>());
		Hedgehog::Math::CQuaternion rotPitch = Hedgehog::Math::CQuaternion::FromTwoVectors(dirXZ.head<3>(), dir.head<3>());

		m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(rotPitch * rotYaw, m_position);
		m_spMatrixNodeTransform->NotifyChanged();
	}
	else
	{
		m_spMatrixNodeTransform->m_Transform.SetPosition(m_position);
		m_spMatrixNodeTransform->NotifyChanged();
	}
}

void CObjWeapon::ResetWeaponData()
{
	for (WeaponData& data : m_weaponData)
	{
		data.Reset();
	}
}

bool CObjWeapon::CanShoot()
{
	return m_type != WT_COUNT && m_weaponData[m_type].m_ammo > 0;
}

CObjWeapon::CObjWeapon
(
	boost::shared_ptr<hh::mr::CMatrixNode> parent
)
	: m_spNodeParent(parent)
	, m_pData(&GetWeaponData(m_type))
{
}

void CObjWeapon::Shoot()
{
	if (!CanShoot())
	{
		return;
	}

	m_pData->m_ammo--;
	
	// change animation
	ChangeState("AirLoop");
	m_spAnimPose->Update(0.001f); // force update bone position
	Common::SonicContextChangeAnimation(AnimationSetPatcher::WeaponAirLoop[m_type]);

	// shoot projectile
	auto node = m_spModel->GetNode("WeaponEffect");
	hh::mr::CTransform startTrans;
	startTrans.m_Rotation = node->GetWorldMatrix().rotation();
	startTrans.m_Position = node->GetWorldMatrix().translation();

	hh::math::CVector targetPos = hh::math::CVector::Zero();
	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	if (context->m_HomingAttackTargetActorID)
	{
		SendMessageImm(context->m_HomingAttackTargetActorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPos));
	}

	m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<CObjProjectile>(m_type, startTrans, targetPos));
}

void CObjWeapon::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CGameObject3D::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);
	Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
	in_pGameDocument->AddUpdateUnit("0", this);

	// model
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(m_pData->m_modelName.c_str(), 0);
	m_spNodeModel = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModel->SetParent(m_spNodeParent.get());
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spNodeModel);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, true);

	// animations
	std::string const air_l = m_pData->m_modelName + "_air_l";
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, m_pData->m_modelName.c_str());
	std::vector<hh::anim::SMotionInfo> entries = std::vector<hh::anim::SMotionInfo>(0, { "","" });
	entries.push_back(hh::anim::SMotionInfo("AirLoop", air_l.c_str(), 2.0f, hh::anim::eMotionRepeatType_PlayOnce));
	m_spAnimPose->AddMotionInfo(&entries.front(), entries.size());
	m_spAnimPose->CreateAnimationCache();
	m_spModel->BindPose(m_spAnimPose);

	// states
	SetContext(this);
	AddAnimationState("AirLoop");

	// initial fire
	Shoot();
}

void CObjWeapon::KillCallback()
{
	NextGenShadow::m_weaponSingleton.reset();
}

bool CObjWeapon::ProcessMessage
(
	Hedgehog::Universe::Message& message,
	bool flag
)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgRestartStage>() || message.Is<Sonic::Message::MsgStageClear>())
		{
			Kill();
			return true;
		}
	}

	return Sonic::CGameObject3D::ProcessMessage(message, flag);
}

void CObjWeapon::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	m_spAnimPose->Update(in_rUpdateInfo.DeltaTime);
	Update(in_rUpdateInfo);
}
