#include "CObjWeapon.h"
#include "Character/NextGenPhysics.h"
#include "Character/NextGenShadow.h"
#include "Utils/AnimationSetPatcher.h"

WeaponType CObjWeapon::m_type = WT_COUNT;
bool CObjWeapon::m_infiniteAmmo = false;
float CObjWeapon::m_darkMeter = 0.0f;
SharedPtrTypeless pfxHandle_awakeDark;

float const cWeapon_infiniteAmmoTime = 20.0f;
float const cWeapon_darkMeterAddAmount = 5.0f;
float const cWeapon_projectileLifeTime = 3.0f;

std::vector<WeaponData> CObjWeapon::m_weaponData =
{
	// EggPawnGun
	{ 
		/*m_weaponModelName*/		"weapon_eggpawngun",
		/*m_projectileModelName*/	"",
		/*m_projectileEffectName*/	"ef_wpn_eggpanwgun_bullet",
		/*m_chargeEffectName*/		"ef_wpn_eggpanwgun_omen",
		/*m_muzzleEffectName*/		"ef_wpn_eggpanwgun_muzzle",
		/*m_hitEffectName*/			"ef_projectile_impact",

		/*m_maxAmmo*/		20, 20,
		/*m_spriteIndex*/	1,
		/*m_chargeTime*/	0.0f,
		/*m_shootInterval*/	0.1f,
		/*m_speed*/			40.0f,
		/*m_gravity*/		0.0f,
		/*m_radius*/		0.125f,
		/*m_autoFire*/		false,

		/*m_chargeSfx*/	0, 
		/*m_shootSfx*/	200616000,
		/*m_hitSfx*/	200616001,

		/*m_modelIndices*/	{4},
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
	, m_positionPrev(startTrans.m_Position)
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

void CObjProjectile::SetAddUpdateUnit
(
	Sonic::CGameDocument* in_pGameDocument
)
{
	in_pGameDocument->AddUpdateUnit("1", this);
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

	return true;
}

bool CObjProjectile::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	uint32_t const typeEnemy = *(uint32_t*)0x1E5E7E8;
	uint32_t const typeBreakable = *(uint32_t*)0x1E5E77C;
	uint32_t const typeTerrain = *(uint32_t*)0x1E5E754;
	uint64_t const bitfield = (1llu << typeEnemy) | (1llu << typeBreakable) | (1llu << typeTerrain);
	m_collisionID = Common::MakeCollisionID(0, bitfield);

	hk2010_2_0::hkpSphereShape* bodyEventTrigger = new hk2010_2_0::hkpSphereShape(m_pData->m_radius);
	AddEventCollision("Damage", bodyEventTrigger, *reinterpret_cast<int*>(0x1E0AF84), true, m_spMatrixNodeTransform); // SpikeAttack
	AddEventCollision("Terrain", bodyEventTrigger, *reinterpret_cast<int*>(0x1E0AFAC), true, m_spMatrixNodeTransform); // BasicAndTerrainCheck

	return true;
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
				hh::math::CVector outPos = hh::math::CVector::Zero();
				hh::math::CVector outNormal = hh::math::CVector::UnitY();
				if (m_lifetime > 0.0f && Common::fRaycast(m_positionPrev, m_position, outPos, outNormal, m_collisionID))
				{
					m_spMatrixNodeTransform->m_Transform.SetPosition(outPos);
					m_spMatrixNodeTransform->NotifyChanged();
				}

				auto effectNode = boost::make_shared<Sonic::CMatrixNodeTransform>();
				effectNode->SetParent(Sonic::CApplicationDocument::GetInstance()->m_pMember->m_spMatrixNodeRoot.get());
				effectNode->m_Transform.SetPosition(m_spMatrixNodeTransform->m_Transform.m_Position);
				effectNode->NotifyChanged();
				m_pGlitterPlayer->PlayOneshot(effectNode, m_pData->m_hitEffectName.c_str(), 1.0f, 1);
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
	if (m_lifetime > cWeapon_projectileLifeTime)
	{
		Kill();
		return;
	}

	UpdateTransform(in_rUpdateInfo.DeltaTime);
}

void CObjProjectile::UpdateTransform(float dt)
{
	m_positionPrev = m_position;
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
	m_darkMeter = 0.0f;
	ToggleInfiniteAmmo(false);

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
		SetWeaponType(WT_COUNT, false);
	}
}

bool CObjWeapon::CanShoot()
{
	return m_type != WT_COUNT && m_weaponData[m_type].m_ammo > 0;
}

void CObjWeapon::AddDarkMeter()
{
	if (m_type == WT_COUNT) return;

	m_darkMeter = min(100.0f, m_darkMeter + cWeapon_darkMeterAddAmount);
	S06HUD_API::SetGadgetHP(m_darkMeter);
}

void CObjWeapon::ToggleInfiniteAmmo(bool enabled)
{
	if (enabled && m_type != WT_COUNT)
	{
		m_infiniteAmmo = true;

		SharedPtrTypeless voiceHandle;
		Common::SonicContextPlayVoice(voiceHandle, 3002036, 20);

		// Refill current weapon to full
		m_weaponData[m_type].m_ammo = m_weaponData[m_type].m_maxAmmo;
		S06HUD_API::SetGadgetCount(m_weaponData[m_type].m_ammo, m_weaponData[m_type].m_maxAmmo);

		SharedPtrTypeless sfx;
		Common::PlaySoundStatic(sfx, 200616003);

		void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
		Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle_awakeDark, matrixNode, "ef_ch_sh_awakedark", 1);
	}
	else
	{
		m_infiniteAmmo = false;
		if (pfxHandle_awakeDark)
		{
			Common::fCGlitterEnd(*PLAYER_CONTEXT, pfxHandle_awakeDark, false);
			pfxHandle_awakeDark = nullptr;
		}
	}
}

void CObjWeapon::SetWeaponType(WeaponType type, bool updateHUD)
{
	if (m_type == type) return;

	if (NextGenShadow::m_weaponSingleton)
	{
		NextGenShadow::m_weaponSingleton->Kill();
		NextGenShadow::m_weaponSingleton.reset();
	}

	if (updateHUD)
	{
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
			S06HUD_API::SetGadgetHP(m_darkMeter);
		}
	}

	SetModelVisibility(m_type, type);
	m_type = type;

	if (type == WT_COUNT)
	{
		// Revert pulley animation
		WRITE_MEMORY(0xE4626D, uint8_t, 0xA1, 0xDC, 0x45, 0xA4, 0x01, 0x50);

		// Enable boost
		NextGenPhysics::toggleBoost(true);

		// Stop infinite ammo
		ToggleInfiniteAmmo(false);
	}
	else
	{
		// Set pulley to use UpReel animation
		WRITE_MEMORY(0xE4626D, uint8_t, 0x68, 0xEC, 0x8A, 0x5F, 0x01, 0x90);

		// Disable boost
		NextGenPhysics::toggleBoost(false);

		// Reload sfx
		SharedPtrTypeless sfx;
		Common::PlaySoundStatic(sfx, 200616003);

		auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
		auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("LeftHand");
		NextGenShadow::m_weaponSingleton = boost::make_shared<CObjWeapon>(attachBone);
		context->m_pPlayer->m_pMember->m_pGameDocument->AddGameObject(NextGenShadow::m_weaponSingleton);
	}

}

void CObjWeapon::SetModelVisibility(WeaponType oldType, WeaponType newType)
{
	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	if (!context || !context->m_pPlayer->m_spCharacterModel) return;

	auto const& model = context->m_pPlayer->m_spCharacterModel;
	bool const chaosBoost = NextGenShadow::m_chaosBoostLevel > 0;

	if (oldType != WT_COUNT)
	{
		WeaponData const& data = m_weaponData[oldType];
		for (uint32_t i = 2; i < model->m_spModel->m_NodeGroupModels.size(); i += 2)
		{
			bool const showForWeapon = data.m_modelIndices.count(i);
			model->m_spModel->m_NodeGroupModels[i]->m_Visible = !showForWeapon && !chaosBoost && NextGenShadow::IsModelVisible();
			model->m_spModel->m_NodeGroupModels[i + 1]->m_Visible = !showForWeapon && chaosBoost && NextGenShadow::IsModelVisible();
		}
	}

	if (newType == WT_COUNT)
	{
		for (uint32_t i = 2; i < model->m_spModel->m_NodeGroupModels.size(); i += 2)
		{
			bool const defaultHand = i == 2;
			model->m_spModel->m_NodeGroupModels[i]->m_Visible = defaultHand && !chaosBoost && NextGenShadow::IsModelVisible();
			model->m_spModel->m_NodeGroupModels[i + 1]->m_Visible = defaultHand && chaosBoost && NextGenShadow::IsModelVisible();
		}
	}
	else
	{
		WeaponData const& data = m_weaponData[newType];
		for (uint32_t i = 2; i < model->m_spModel->m_NodeGroupModels.size(); i += 2)
		{
			bool const showForWeapon = data.m_modelIndices.count(i);
			model->m_spModel->m_NodeGroupModels[i]->m_Visible = showForWeapon && !chaosBoost && NextGenShadow::IsModelVisible();
			model->m_spModel->m_NodeGroupModels[i + 1]->m_Visible = showForWeapon && chaosBoost && NextGenShadow::IsModelVisible();
		}
	}
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

void CObjWeapon::SetAddUpdateUnit
(
	Sonic::CGameDocument* in_pGameDocument
)
{
	in_pGameDocument->AddUpdateUnit("1", this);
}

bool CObjWeapon::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// model
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(m_pData->m_weaponModelName.c_str(), 0);
	m_spNodeModel = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModel->SetParent(m_spNodeParent.get());
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spNodeModel);
	m_spNodeMuzzle = m_spModel->GetNode("WeaponEffect");
	Sonic::CGameObject::AddRenderable("Object", m_spModel, true);

	// bone rotation for aiming
	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	FUNCTION_PTR(uint16_t, __thiscall, GetBoneIDFromAnimationPose, 0x6C7CC0, hh::anim::CAnimationPose* pAnimPose, hh::base::CSharedString const& name);
	m_rotateBoneData.m_boneID = GetBoneIDFromAnimationPose(context->m_pPlayer->m_spAnimationPose.get(), "Spine");
	
	return true;
}

void CObjWeapon::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	// undo -90 X-rotation on Shadow's root
	hh::math::CQuaternion const rotation = Eigen::AngleAxisf(PI_F * 0.5f, hh::math::CVector::UnitX()) * hh::math::CQuaternion::Identity();
	m_spNodeModel->m_Transform.SetRotation(rotation);
	m_spNodeModel->NotifyChanged();

	// spawned during Chaos Snap teleport, hide
	if (!NextGenShadow::IsModelVisible())
	{
		SendMessageImm(m_ActorID, boost::make_shared<Sonic::Message::MsgSetVisible>(false));
	}
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
			SetWeaponType(WT_COUNT);
			return true;
		}

		if (message.Is<Sonic::Message::MsgSetVisible>())
		{
			auto& msg = static_cast<Sonic::Message::MsgSetVisible&>(message);
			if (pfxHandle_awakeDark && !msg.m_Visible)
			{
				// hide awakeDark effect immediately
				Common::fCGlitterEnd(*PLAYER_CONTEXT, pfxHandle_awakeDark, true);
				pfxHandle_awakeDark = nullptr;
			}
			else if (m_infiniteAmmo && !pfxHandle_awakeDark && msg.m_Visible)
			{
				// resume awakeDark effect but without initial flare
				void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
				Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle_awakeDark, matrixNode, "ef_ch_sh_awakedark_loop", 1);
			}
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void CObjWeapon::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	std::lock_guard<std::mutex> guard(m_mutex);

	if (m_infiniteAmmo)
	{
		m_darkMeter = max(0.0f, m_darkMeter - 100.0f * in_rUpdateInfo.DeltaTime / cWeapon_infiniteAmmoTime);
		S06HUD_API::SetGadgetHP(m_darkMeter);

		if (m_darkMeter == 0.0f || NextGenShadow::m_chaosBoostLevel == 0)
		{
			ToggleInfiniteAmmo(false);
		}
	}
	else if (m_darkMeter == 100.0f && NextGenShadow::m_chaosBoostLevel > 0)
	{
		ToggleInfiniteAmmo(true);
	}

	// make sure dummy transform follows the model
	m_spMatrixNodeTransform->m_Transform.SetPosition(m_spNodeModel->GetWorldMatrix().translation());
	m_spMatrixNodeTransform->NotifyChanged();

	switch (m_state)
	{
	case State::AirCharge:
	{
		m_chargeTimer -= in_rUpdateInfo.DeltaTime;
		if (m_chargeTimer <= 0.0f)
		{
			if (m_chargeID)
			{
				m_pGlitterPlayer->StopByID(m_chargeID, false);
				m_chargeID = 0;
			}

			m_state = State::AirFire;
			m_shootTimer = 0.0f;
		}
		break;
	}
	case State::AirFire:
	{
		if (m_shootTimer <= 0.0f)
		{
			m_shootTimer = m_pData->m_shootInterval;
			if (CanShoot())
			{
				Shoot();
			}
			else
			{
				// no ammo sfx
				SharedPtrTypeless sfx;
				Common::ObjectPlaySound(this, 200616002, sfx);
				m_shootTimer = 0.5f;
			}

			m_shootBuffered = false;
			m_state = State::Cooldown;
			m_cooldownTimer = 0.2f;
		}
		break;
	}
	case State::Cooldown:
	{
		m_shootTimer -= in_rUpdateInfo.DeltaTime;
		m_cooldownTimer -= in_rUpdateInfo.DeltaTime;

		Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
		if (!m_pData->m_autoFire && !m_shootBuffered)
		{
			m_shootBuffered = padState->IsTapped(Sonic::EKeyState::eKeyState_RightTrigger);
		}

		if (m_shootTimer <= 0.0f && ((m_pData->m_autoFire && padState->IsDown(Sonic::EKeyState::eKeyState_RightTrigger)) || m_shootBuffered))
		{
			m_state = State::AirFire;
		}
		break;
	}
	}
}

bool CObjWeapon::IsActive() const
{
	std::lock_guard<std::mutex> guard(m_mutex);
	return m_state != State::Idle;
}

bool CObjWeapon::CanRelease() const
{
	std::lock_guard<std::mutex> guard(m_mutex);
	return m_state == State::Cooldown && m_cooldownTimer <= 0.0f && m_shootTimer <= 0.0f;
}

void CObjWeapon::SetStateIdle()
{
	std::lock_guard<std::mutex> guard(m_mutex);

	m_state = State::Idle;
	if (m_chargeID)
	{
		m_pGlitterPlayer->StopByID(m_chargeID, false);
		m_chargeID = 0;
	}
}

void CObjWeapon::SetActive(WeaponFireType type)
{
	std::lock_guard<std::mutex> guard(m_mutex);

	m_shootTimer = 0.0f;
	m_chargeTimer = m_pData->m_chargeTime;
	if (m_chargeTimer > 0.0f)
	{
		if (!m_pData->m_chargeEffectName.empty())
		{
			m_chargeID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spNodeMuzzle, m_pData->m_chargeEffectName.c_str(), 1.0f);
		}

		if (m_pData->m_chargeSfx)
		{
			SharedPtrTypeless sfx;
			Common::ObjectPlaySound(this, m_pData->m_chargeSfx, sfx);
		}
	}

	switch (type)
	{
	case WFT_Stand:
		// TODO:
	case WFT_Run:
		// TODO:
	case WFT_Air:
		m_state = State::AirCharge;
		Common::SonicContextChangeAnimation(AnimationSetPatcher::WeaponAirLoop[m_type]);
		break;
	}
}

void CObjWeapon::UpdateBoneRotation()
{
	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	hh::math::CVector targetPosition = hh::math::CVector::Zero();
	if (m_state != State::Idle && context->m_HomingAttackTargetActorID)
	{
		SendMessageImm(context->m_HomingAttackTargetActorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));
	}

	hh::math::CQuaternion targetRotation = hh::math::CQuaternion::Identity();
	if (!targetPosition.isZero())
	{
		hh::math::CVector dir = (context->m_spMatrixNode->m_Transform.m_Rotation.conjugate() * (targetPosition - m_spNodeMuzzle->GetWorldMatrix().translation())).normalized();
		hh::math::CVector dirXZ = dir; dirXZ.y() = 0.0f; dirXZ.normalize();
		
		float yaw = min(acosf(hh::math::CVector::UnitZ().dot(dirXZ)), 80.0f * DEG_TO_RAD);
		if (dir.dot(Eigen::Vector3f::UnitX()) < 0) yaw = -yaw;
		float pitch = min(acosf(dir.dot(dirXZ)), 70.0f * DEG_TO_RAD);
		if (dir.dot(Eigen::Vector3f::UnitY()) < 0) pitch = -pitch;

		targetRotation = Eigen::AngleAxisf(yaw, hh::math::CVector::UnitX()) * Eigen::AngleAxisf(pitch, hh::math::CVector::UnitZ());
	}
	m_rotateBoneData.m_addRotation = m_rotateBoneData.m_addRotation.slerp(0.1f, targetRotation);

	FUNCTION_PTR(Hedgehog::Animation::hkQsTransform*, __thiscall, GetHkQsTransform, 0x833390, hh::anim::SAnimData* pAnimData, int id);
	Hedgehog::Animation::hkQsTransform* pTrans = GetHkQsTransform(context->m_pPlayer->m_spAnimationPose->m_pAnimData, m_rotateBoneData.m_boneID);
	pTrans->m_Rotation = (m_rotateBoneData.m_addRotation * pTrans->m_Rotation).normalized();
}

void CObjWeapon::Shoot()
{
	if (!m_infiniteAmmo)
	{
		m_pData->m_ammo--;
		S06HUD_API::SetGadgetCount(m_pData->m_ammo, m_pData->m_maxAmmo);
	}

	// muzzle
	if (!m_pData->m_muzzleEffectName.empty())
	{
		m_pGlitterPlayer->PlayOneshot(m_spNodeMuzzle, m_pData->m_muzzleEffectName.c_str(), 1.0f, 1);
	}

	auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	Common::SonicContextChangeAnimation(AnimationSetPatcher::WeaponAirFire[m_type]);

	// shoot projectile
	hh::mr::CTransform startTrans;
	startTrans.m_Rotation = context->m_spMatrixNode->m_Transform.m_Rotation;
	startTrans.m_Position = m_spNodeMuzzle->GetWorldMatrix().translation();

	hh::math::CVector targetPos = hh::math::CVector::Zero();
	if (context->m_HomingAttackTargetActorID)
	{
		SendMessageImm(context->m_HomingAttackTargetActorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPos));
	}

	m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<CObjProjectile>(m_type, startTrans, targetPos));
}
