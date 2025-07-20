#include "Explosion.h"

#include "Managers/ScoreManager.h"

float const cExplosion_radius = 5.0f;
float const cExplosion_duration = 0.3f;
float const cExplosion_velocityObjPhy = 20.0f;
float const cExplosion_velocityEnemy = 10.0f;

float Explosion::GetDefaultRadius()
{
    return cExplosion_radius;
}

bool Explosion::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// set initial position
	m_spMatrixNodeTransform->m_Transform.SetPosition(m_position);
	m_spMatrixNodeTransform->NotifyChanged();

	// set up collision with enemy
	m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());

	hk2010_2_0::hkpSphereShape* shapeEventTrigger1 = new hk2010_2_0::hkpSphereShape(m_radius);
	AddEventCollision("Damage", shapeEventTrigger1, *reinterpret_cast<int*>(0x1E0AF84), true, m_spNodeEventCollision); // SpikeAttack

	return true;
}

bool Explosion::ProcessMessage(Hedgehog::Universe::Message& message, bool flag)
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
            uint32_t enemyType = 0u;
            SendMessageImm(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgGetEnemyType>(&enemyType));

            auto* senderMessageActor = m_pMessageManager->GetMessageActor(message.m_SenderActorID);
            uint32_t* senderActor = (uint32_t*)((uint32_t)senderMessageActor - 0x28);
            bool isObjectPhysics = *(uint32_t*)senderMessageActor == 0x16CF58C;

            hh::math::CVector targetPosition = hh::math::CVector::Identity();
            if (enemyType > 0)
            {
                // try to get center position from lock-on for enemy
                SendMessageImm(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));
            }
            else if (isObjectPhysics)
            {
                // get dynamic position for object physics
                Common::fObjectPhysicsDynamicPosition(senderActor, targetPosition);
            }
            else
            {
                SendMessageImm(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgGetPosition>(&targetPosition));
            }

            // apply damage
            if (!targetPosition.isIdentity())
            {
                SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
                    (
                        *(uint32_t*)0x1E0BE34, // DamageID_NoAttack
                        m_position,
                        (targetPosition - m_position) * (enemyType > 0 ? cExplosion_velocityEnemy : cExplosion_velocityObjPhy)
                    ));

                if (enemyType > 0)
                {
                    ScoreManager::addEnemyChain(senderActor);
                }
            }

            return true;
        }
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void Explosion::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
    // time out
    m_lifeTime += in_rUpdateInfo.DeltaTime;
    if (m_lifeTime >= cExplosion_duration)
    {
        Kill();
    }
}
