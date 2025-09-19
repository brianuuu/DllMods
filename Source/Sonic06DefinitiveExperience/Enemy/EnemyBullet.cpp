#include "EnemyBullet.h"

float const c_enemyBulletSpeed = 60.0f;
float const c_enemyBulletLifetime = 2.0f;
hh::math::CVector const c_enemyBulletHead = hh::math::CVector(0.0f, 0.0f, 0.2f);

EnemyBullet::EnemyBullet
(
	uint32_t owner, 
	hh::mr::CTransform const& startTrans
)
	: m_owner(owner)
{
	// initial transform
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(startTrans.m_Rotation, startTrans.m_Position);
	m_spMatrixNodeTransform->NotifyChanged();

	m_spNodeHead = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeHead->m_Transform.SetPosition(c_enemyBulletHead);
	m_spNodeHead->NotifyChanged();
	m_spNodeHead->SetParent(m_spMatrixNodeTransform.get());
	m_headPositionPrev = m_spNodeHead->GetWorldMatrix().translation();
}

bool EnemyBullet::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// model
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData("en_cmn_bullet", 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, true);

	return true;
}

bool EnemyBullet::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// damage to object
	uint32_t const typeEnemy = *(uint32_t*)0x1E5E7E8;
	uint32_t const typeBreakable = *(uint32_t*)0x1E5E77C;
	uint32_t const typeTerrain = *(uint32_t*)0x1E5E754;
	uint64_t const bitfield = (1llu << typeEnemy) | (1llu << typeBreakable) | (1llu << typeTerrain);
	m_collisionID = Common::MakeCollisionID(0, bitfield);
	hk2010_2_0::hkpCapsuleShape* bodyEventTrigger = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(0.0f, 0.0f, 0.1f), hh::math::CVector(0.0f, 0.0f, -0.3f), 0.08f);
	AddEventCollision("Attack", bodyEventTrigger, m_collisionID, true, m_spMatrixNodeTransform);

	return true;
}

bool EnemyBullet::ProcessMessage
(
	Hedgehog::Universe::Message& message, bool flag
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
			if (message.m_SenderActorID != m_owner)
			{
				hh::math::CVector outPos = hh::math::CVector::Zero();
				hh::math::CVector outNormal = hh::math::CVector::UnitY();
				if (m_lifetime > 0.0f && Common::fRaycast(m_headPositionPrev, m_spNodeHead->GetWorldMatrix().translation(), outPos, outNormal, m_collisionID))
				{
					m_spMatrixNodeTransform->m_Transform.SetPosition(outPos);
					m_spMatrixNodeTransform->NotifyChanged();
				}
				m_pGlitterPlayer->PlayOneshot(m_spMatrixNodeTransform, "ef_bullet_hit", 1.0f, 1);

				SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
					(
						*(uint32_t*)0x1E0BE28, // DamageID_SonicHeavy
						m_spMatrixNodeTransform->m_Transform.m_Position,
						m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ() * c_enemyBulletSpeed
					)
				);

				Kill();
			}
			return true;
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void EnemyBullet::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	m_lifetime += in_rUpdateInfo.DeltaTime;
	if (m_lifetime > c_enemyBulletLifetime)
	{
		Kill();
		return;
	}

	m_headPositionPrev = m_spNodeHead->GetWorldMatrix().translation();
	hh::math::CVector const newPosition = m_spMatrixNodeTransform->m_Transform.m_Position + m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ() * c_enemyBulletSpeed * in_rUpdateInfo.DeltaTime;
	m_spMatrixNodeTransform->m_Transform.SetPosition(newPosition);
	m_spMatrixNodeTransform->NotifyChanged();
}
