#include "DarkSphere.h"

float const c_darkSphereRadiusS = 0.6f;
float const c_darkSphereRadiusL = 1.2f;
float const c_darkSphereCenterRadiusS = 0.2f;
float const c_darkSphereCenterRadiusL = 0.4f;
float const c_darkSphereLifetimeS = 5.0f;
float const c_darkSphereLifetimeL = 10.0f;
float const c_darkSphereTurnRateS = 60.0f * DEG_TO_RAD;
float const c_darkSphereTurnRateL = 180.0f * DEG_TO_RAD;

DarkSphere::DarkSphere
(
	uint32_t owner, 
	uint32_t target, 
	float speed,
	bool isLarge,
	hh::math::CVector const& startPos
)
	: m_owner(owner)
	, m_target(target)
	, m_speed(speed)
	, m_isLarge(isLarge)
{
	// initial transform
	m_spMatrixNodeTransform->m_Transform.SetPosition(startPos);
	m_spMatrixNodeTransform->NotifyChanged();
}

bool DarkSphere::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	m_effectID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spMatrixNodeTransform, m_isLarge ? "ef_mephiles_sphere_l" : "ef_mephiles_sphere_s", 1.0f);
	return true;
}

bool DarkSphere::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	uint32_t const typePlayer = *(uint32_t*)0x1E5E788;
	uint32_t const typeInsulate = *(uint32_t*)0x1E5E780;
	uint32_t const typeTerrain = *(uint32_t*)0x1E5E754;

	hk2010_2_0::hkpSphereShape* playerEventTrigger = new hk2010_2_0::hkpSphereShape(m_isLarge ? c_darkSphereRadiusL : c_darkSphereRadiusS);
	AddEventCollision("Player", playerEventTrigger, Common::MakeCollisionID(0, (1llu << typePlayer)), true, m_spMatrixNodeTransform);

	hk2010_2_0::hkpSphereShape* terrainEventTrigger = new hk2010_2_0::hkpSphereShape(m_isLarge ? c_darkSphereCenterRadiusL : c_darkSphereCenterRadiusS);
	AddEventCollision("Terrain", terrainEventTrigger, Common::MakeCollisionID(0, (1llu << typeInsulate) | (1llu << typeTerrain)), true, m_spMatrixNodeTransform);

	return true;
}

void DarkSphere::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	// large sphere only starts moving from event
	if (!m_isLarge)
	{
		SetInitialVelocity();
	}
	else
	{
		Common::ObjectPlaySound(this, 200615008, m_largeSfx);
	}
}

bool DarkSphere::ProcessMessage
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
					hh::math::CVector::Zero()
				)
			);

			if (message.m_SenderActorID == m_target)
			{
				// TODO: nofity owner MsgHitEnemyShot
			}

			Explode();
			return true;
		}

		if (message.Is<Sonic::Message::MsgNotifyObjectEvent>() && m_isLarge)
		{
			auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
			switch (msg.m_Event)
			{
			case 6:
			{
				SetInitialVelocity();

				m_largeSfx.reset();
				Common::ObjectPlaySound(this, 200615009, m_largeSfx);
				break;
			}
			case 7:
			{
				Explode();
				break;
			}
			}
			return true;
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void DarkSphere::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	if (m_velocity.isZero()) return;

	m_lifetime += in_rUpdateInfo.DeltaTime;
	if (m_lifetime > (m_isLarge ? c_darkSphereLifetimeL : c_darkSphereLifetimeS))
	{
		Explode();
		return;
	}

	hh::math::CVector const targetPos = GetTargetPos();
	hh::math::CVector newDirection = (targetPos - m_spMatrixNodeTransform->m_Transform.m_Position).normalized();
	hh::math::CVector oldDirection = m_velocity.normalized();
	float dot = oldDirection.dot(newDirection);
	Common::ClampFloat(dot, -1.0f, 1.0f);

	float const angle = acos(dot);
	float const maxAngle = in_rUpdateInfo.DeltaTime * (m_isLarge ? c_darkSphereTurnRateL : c_darkSphereTurnRateS);
	if (angle > maxAngle)
	{
		hh::math::CVector cross = oldDirection.cross(newDirection).normalized();
		Eigen::AngleAxisf rot(maxAngle, cross);
		newDirection = rot * oldDirection;
	}

	m_velocity = newDirection * m_speed;
	hh::math::CVector const newPosition = m_spMatrixNodeTransform->m_Transform.m_Position + m_velocity * in_rUpdateInfo.DeltaTime;
	m_spMatrixNodeTransform->m_Transform.SetPosition(newPosition);
	m_spMatrixNodeTransform->NotifyChanged();

	if (m_largeSfx)
	{
		hh::math::CVector* pSoundHandle = (hh::math::CVector*)m_largeSfx.get();
		pSoundHandle[2] = newPosition;
	}
}

hh::math::CVector DarkSphere::GetTargetPos() const
{
	hh::math::CVector targetPos = hh::math::CVector::Zero();
	SendMessageImm(m_target, Sonic::Message::MsgGetPosition(targetPos));
	targetPos.y() += 0.5f; // player height adjustment

	return targetPos;
}

void DarkSphere::SetInitialVelocity()
{
	hh::math::CVector const targetPos = GetTargetPos();
	if (!m_spMatrixNodeTransform->m_Transform.m_Position.isApprox(targetPos))
	{
		m_velocity = (targetPos - m_spMatrixNodeTransform->m_Transform.m_Position).normalized() * m_speed;
	}
}

void DarkSphere::Explode()
{
	SharedPtrTypeless soundHandle;
	Common::ObjectPlaySound(this, m_isLarge ? 200614015 : 200614014, soundHandle);

	if (m_largeSfx)
	{
		m_largeSfx.reset();
	}

	m_pGlitterPlayer->PlayOneshot(m_spMatrixNodeTransform, m_isLarge ? "ef_mephiles_spherebomb_l" : "ef_mephiles_spherebomb_s", 1.0f, 1);

	SendMessage(m_owner, boost::make_shared<Sonic::Message::MsgNotifyObjectEvent>(2));

	Kill();
}
