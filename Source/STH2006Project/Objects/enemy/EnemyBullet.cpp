#include "EnemyBullet.h"

float const c_enemyBulletSpeed = 60.0f;
float const c_enemyBulletLifetime = 2.0f;

bool EnemyBullet::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// initial transform
	m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(m_startTrans.m_Rotation, m_startTrans.m_Position);
	m_spMatrixNodeTransform->NotifyChanged();

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
	hk2010_2_0::hkpCapsuleShape* bodyEventTrigger = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(0.0f, 0.0f, 0.1f), hh::math::CVector(0.0f, 0.0f, -0.3f), 0.08f);
	AddEventCollision("Attack", bodyEventTrigger, Common::MakeCollisionID(0, bitfield), true, m_spMatrixNodeTransform);

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
			if (message.m_SenderActorID != m_owner && m_frames > 1)
			{
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

	return Sonic::CGameObject3D::ProcessMessage(message, flag);
}

void EnemyBullet::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	m_frames++;
	m_lifetime += in_rUpdateInfo.DeltaTime;
	if (m_lifetime > c_enemyBulletLifetime)
	{
		Kill();
		return;
	}

	hh::math::CVector const newPosition = m_spMatrixNodeTransform->m_Transform.m_Position + m_spMatrixNodeTransform->m_Transform.m_Rotation * hh::math::CVector::UnitZ() * c_enemyBulletSpeed * in_rUpdateInfo.DeltaTime;
	m_spMatrixNodeTransform->m_Transform.SetPosition(newPosition);
	m_spMatrixNodeTransform->NotifyChanged();
}
