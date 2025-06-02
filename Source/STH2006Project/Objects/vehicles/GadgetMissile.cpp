#include "GadgetMissile.h"

float const c_gadgetMissileSpeed = 60.0f;
float const c_gadgetMissileLifetime = 5.0f;
float const c_gadgetMissileTurnRate = 90.0f * DEG_TO_RAD;

bool GadgetMissile::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// initial transform
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(m_startTrans.m_Rotation, m_startTrans.m_Position);
	m_spMatrixNodeTransform->NotifyChanged();
	m_velocity = m_startTrans.m_Rotation * hh::math::CVector::UnitZ() * c_gadgetMissileSpeed;

	// model
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData("Gadget_Missile", 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, true);

	// effect
	m_spNodeEffect = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeEffect->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.0f, -0.34f));
	m_spNodeEffect->NotifyChanged();
	m_spNodeEffect->SetParent(m_spMatrixNodeTransform.get());
	m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spNodeEffect, "ef_missile_smoke", 1.0f);
	m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spNodeEffect, "ef_missile_jet", 1.0f);

	return true;
}

bool GadgetMissile::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// damage to object
	uint32_t const typeEnemy = *(uint32_t*)0x1E5E7E8;
	uint32_t const typeBreakable = *(uint32_t*)0x1E5E77C;
	uint32_t const typeTerrain = *(uint32_t*)0x1E5E754;
	uint64_t const bitfield = (1llu << typeEnemy) | (1llu << typeBreakable) | (1llu << typeTerrain);
	hk2010_2_0::hkpCapsuleShape* bodyEventTrigger = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(0.0f, 0.0f, 0.31f), hh::math::CVector(0.0f, 0.0f, -0.33f), 0.1f);
	AddEventCollision("Attack", bodyEventTrigger, Common::MakeCollisionID(0, bitfield), true, m_spMatrixNodeTransform);

	// enemy search collision
	hk2010_2_0::hkpCapsuleShape* searchEventTrigger = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(0.0f, 0.0f, 20.0f), hh::math::CVector(0.0f, 0.0f, 40.0f), 20.0f);
	AddEventCollision("Search", searchEventTrigger, *(uint32_t*)0x1E0AF54, true, m_spMatrixNodeTransform); // ColID_TypeEnemy

	return true;
}

bool GadgetMissile::ProcessMessage(Hedgehog::Universe::Message& message, bool flag)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgHitEventCollision>())
		{
			auto& msg = static_cast<Sonic::Message::MsgHitEventCollision&>(message);
			if (msg.m_Symbol == "Attack")
			{
				if (message.m_SenderActorID != m_owner)
				{
					SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
						(
							*(uint32_t*)0x1E0BE28, // DamageID_SonicHeavy
							m_spMatrixNodeTransform->m_Transform.m_Position,
							m_velocity
						)
					);

					SharedPtrTypeless sfx;
					Common::ObjectPlaySound(this, 4002048, sfx);
					m_pGlitterPlayer->PlayOneshot(m_spMatrixNodeTransform, "ef_en_com_yh2_explosion", 1.0f, 1);

					Kill();
				}
			}
			else if (msg.m_Symbol == "Search")
			{
				if (std::find(m_targets.begin(), m_targets.end(), msg.m_SenderActorID) == m_targets.end())
				{
					m_targets.push_back(msg.m_SenderActorID);
				}
			}
			return true;
		}

		if (message.Is<Sonic::Message::MsgLeaveEventCollision>())
		{
			auto& msg = static_cast<Sonic::Message::MsgLeaveEventCollision&>(message);
			if (msg.m_Symbol == "Search")
			{
				m_targets.erase(std::remove(m_targets.begin(), m_targets.end(), msg.m_SenderActorID), m_targets.end());
			}
		}
	}

	return Sonic::CGameObject3D::ProcessMessage(message, flag);
}

void GadgetMissile::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	m_lifetime += in_rUpdateInfo.DeltaTime;
	if (m_lifetime > c_gadgetMissileLifetime)
	{
		Kill();
		return;
	}

	hh::math::CVector const currentPosition = m_spMatrixNodeTransform->m_Transform.m_Position;
	if (!m_targets.empty())
	{
		float minDistSq = FLT_MAX;
		hh::math::CVector newDirection = hh::math::CVector::Zero();
		for (uint32_t const targetID : m_targets)
		{
			// get closest target position
			hh::math::CVector targetPosition = hh::math::CVector::Zero();
			SendMessageImm(targetID, boost::make_shared<Sonic::Message::MsgGetPosition>(&targetPosition));
			if (!targetPosition.isZero())
			{
				SendMessageImm(targetID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));
				float const distSq = (targetPosition - currentPosition).squaredNorm();
				if (distSq < minDistSq)
				{
					minDistSq = distSq;
					newDirection = (targetPosition - currentPosition).normalized();
				}
			}
		}

		if (!newDirection.isZero())
		{
			hh::math::CVector oldDirection = m_velocity.normalized();
			float dot = oldDirection.dot(newDirection);
			Common::ClampFloat(dot, -1.0f, 1.0f);

			float const angle = acos(dot);
			float const maxAngle = in_rUpdateInfo.DeltaTime * c_gadgetMissileTurnRate;
			if (angle > maxAngle)
			{
				hh::math::CVector cross = oldDirection.cross(newDirection).normalized();
				Eigen::AngleAxisf rot(maxAngle, cross);
				newDirection = rot * oldDirection;
			}

			m_velocity = newDirection * c_gadgetMissileSpeed;
		}
	}

	// Rotate to correct rotation
	hh::math::CVector const dir = m_velocity.normalized();
	hh::math::CVector dirXZ = dir; dirXZ.y() = 0.0f;
	hh::math::CQuaternion rotYaw = Hedgehog::Math::CQuaternion::FromTwoVectors(Hedgehog::Math::CVector::UnitZ(), dirXZ.head<3>());
	hh::math::CQuaternion rotPitch = Hedgehog::Math::CQuaternion::FromTwoVectors(dirXZ.head<3>(), dir.head<3>());

	hh::math::CVector const newPosition = currentPosition + m_velocity * in_rUpdateInfo.DeltaTime;
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(rotPitch * rotYaw, newPosition);
	m_spMatrixNodeTransform->NotifyChanged();
}
