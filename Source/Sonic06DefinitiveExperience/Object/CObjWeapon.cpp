#include "CObjWeapon.h"
#include "Character/NextGenShadow.h"
#include "Utils/AnimationSetPatcher.h"

WeaponType CObjWeapon::m_type = WT_COUNT; // TODO: default WT_COUNT
std::vector<WeaponData> CObjWeapon::m_weaponData =
{
	// EggPawnGun
	{ 
		"weapon_eggpawngun", 
		50, 50, 1,
		"ef_eggpanwgun_omen", 0.2f,
		20.0f, 0.0f, 0.125f,
		"", "ef_eggpanwgun_bullet",
		"ef_eggpanwgun_muzzle", "ef_projectile_impact",
		0, 0, 0
	},
};

CObjProjectile::CObjProjectile
(
	WeaponType type, 
	hh::mr::CTransform const& startTrans, 
	hh::math::CVector const& targetPos
)
	: m_type(type)
	, m_pData(&CObjWeapon::GetWeaponData(type))
	, m_position(startTrans.m_Position)
{

	// initial velocity
	if (targetPos.isZero())
	{
		m_velocity = startTrans.m_Rotation * hh::math::CVector::UnitZ() * m_pData->m_speed;
		if (m_pData->m_gravity > 0.0f)
		{
			// projectiles with gravity only set initial rotation
			m_spMatrixNodeTransform->m_Transform.SetRotation(startTrans.m_Rotation);
		}
	}
	else
	{
		hh::math::CVector const dir = (targetPos - m_position).normalized();
		hh::math::CVector dirXZ = dir; dirXZ.y() = 0.0f;

		if (m_pData->m_gravity == 0.0f)
		{
			m_velocity = dir * m_pData->m_speed;
		}
		else
		{
			// TODO: calculate initial speed to hit target
			hh::math::CQuaternion rotation = hh::math::CQuaternion::FromTwoVectors(Hedgehog::Math::CVector::UnitZ(), dirXZ.head<3>()) * startTrans.m_Rotation;
			m_velocity = rotation * hh::math::CVector::UnitZ() * m_pData->m_speed;

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

	if (m_pData->m_shootSfx)
	{
		SharedPtrTypeless sfx;
		Common::ObjectPlaySound(this, m_pData->m_shootSfx, sfx);
	}
}

bool CObjProjectile::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// model
	if (!m_pData->m_projectileModelName.empty())
	{
		hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
		boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(m_pData->m_projectileModelName.c_str(), 0);
		m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
		m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
		Sonic::CGameObject::AddRenderable("Object", m_spModel, true);
	}

	// effect
	if (!m_pData->m_projectileEffectName.empty())
	{
		m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spMatrixNodeTransform, m_pData->m_projectileEffectName.c_str(), 1.0f);
	}

	// muzzle
	if (!m_pData->m_muzzleEffectName.empty())
	{
		auto effectNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
		effectNode->SetParent(Sonic::CApplicationDocument::GetInstance()->m_pMember->m_spMatrixNodeRoot.get());
		effectNode->m_Transform.SetPosition(m_spMatrixNodeTransform->m_Transform.m_Position);
		effectNode->NotifyChanged();
		m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, effectNode, m_pData->m_muzzleEffectName.c_str(), 1.0f);
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
	hk2010_2_0::hkpSphereShape* bodyEventTrigger = new hk2010_2_0::hkpSphereShape(m_pData->m_radius);
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

			if (m_pData->m_hitSfx)
			{
				SharedPtrTypeless sfx;
				Common::ObjectPlaySound(this, m_pData->m_hitSfx, sfx);
			}

			if (!m_pData->m_hitEffectName.empty())
			{
				m_pGlitterPlayer->PlayOneshot(m_spMatrixNodeTransform, m_pData->m_hitEffectName.c_str(), 1.0f, 1);
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
	m_lifetime += in_rUpdateInfo.DeltaTime;
	if (m_lifetime > 5.0f)
	{
		Kill();
		return;
	}

	UpdateTransform(in_rUpdateInfo.DeltaTime);
}

void CObjProjectile::UpdateTransform(float dt)
{
	m_velocity.y() -= m_pData->m_gravity * dt;
	m_position += m_velocity * dt;

	if (m_pData->m_gravity == 0.0f)
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

void CObjWeapon::VerifySpriteIndex()
{
	if (m_type == WT_COUNT) return;

	WeaponData const& data = m_weaponData[m_type];
	if (S06HUD_API::GetGadgetSpriteIndex() != data.m_spriteIndex)
	{
		// don't call SetWeaponType, we just want to match HUD
		m_type = WT_COUNT;
	}
}

bool CObjWeapon::CanShoot()
{
	return m_type != WT_COUNT && m_weaponData[m_type].m_ammo > 0;
}

void CObjWeapon::SetWeaponType(WeaponType type)
{
	if (m_type == type) return;

	if (type == WT_COUNT)
	{
		WeaponData const& data = m_weaponData[m_type];
		S06HUD_API::SetGadgetMaxCount(-1, data.m_spriteIndex);
	}
	else
	{
		WeaponData const& data = m_weaponData[type];
		S06HUD_API::SetGadgetMaxCount(data.m_maxAmmo, data.m_spriteIndex);
		S06HUD_API::SetGadgetCount(data.m_ammo, data.m_maxAmmo);
	}

	m_type = type;
}

void CObjWeapon::NextGun()
{
	if (m_type < WT_GunFirst || m_type >= WT_GunLast)
	{
		SetWeaponType(WT_GunFirst);
	}
	else
	{
		SetWeaponType(WeaponType(m_type + 1));
	}
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
	S06HUD_API::SetGadgetCount(m_pData->m_ammo, m_pData->m_maxAmmo);
	
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
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(m_pData->m_weaponModelName.c_str(), 0);
	m_spNodeModel = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModel->SetParent(m_spNodeParent.get());
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spNodeModel);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, true);

	// animations
	std::string const air_l = m_pData->m_weaponModelName + "_air_l";
	m_spAnimPose = boost::make_shared<Hedgehog::Animation::CAnimationPose>(in_spDatabase, m_pData->m_weaponModelName.c_str());
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
