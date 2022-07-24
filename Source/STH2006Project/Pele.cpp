#include "Pele.h"

class CObjPele : public Sonic::CGameObject
{
	enum PeleState
	{
		PS_Wait,
		PS_StandUp,
		PS_Follow,
		PS_SitDown,
	};

	PathDataCollection m_path;
	PathFollowData m_followData;

	void* m_pObject;
	PeleState m_state;
	bool m_triggerFired;

public:
	CObjPele(void* pObject)
	{
		if (!PathManager::parsePathXml(m_path, false, "Assets\\Stage\\Pele.path.xml") == tinyxml2::XML_SUCCESS)
		{
			MessageBox(NULL, L"Failed to parse Pele.path.xml", NULL, MB_ICONERROR);
		}

		if (!m_path.empty())
		{
			m_followData.m_yawOnly = true;
			m_followData.m_speed = 4.0f;
			m_followData.m_loop = false;
			m_followData.m_pPathData = &m_path[0];

			m_followData.m_rotation = Eigen::Quaternionf::Identity();
			m_followData.m_position = m_path[0].m_knots[0].m_point;
		}

		m_pObject = pObject;
		m_state = PeleState::PS_Wait;
		m_triggerFired = false;

		printf("[Pele] Pele the Beloved Dog address 0x%08x\n", (uint32_t)pObject);
	}

	void AddCallback
	(
		const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
		Sonic::CGameDocument* pGameDocument,
		const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
	) override
	{
		Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
		pGameDocument->AddUpdateUnit("0", this);
	}

	bool ProcessMessage
	(
		Hedgehog::Universe::Message& message,
		bool flag
	) override
	{
		if (flag)
		{
			if (std::strstr(message.GetType(), "MsgRestartStage") != nullptr)
			{
				Kill();
				return true;
			}
		}

		return Sonic::CGameObject::ProcessMessage(message, flag);
	}

	void UpdateParallel
	(
		const Hedgehog::Universe::SUpdateInfo& updateInfo
	) override
	{
		// Not object physics? Kill
		if (*(uint32_t*)m_pObject != 0x16CF524)
		{
			Kill();
			return;
		}

		Eigen::Vector3f sonicPosition;
		Eigen::Quaternionf sonicRotation;
		if (!Common::GetPlayerTransform(sonicPosition, sonicRotation))
		{
			return;
		}

		if (m_followData.m_finished && !m_triggerFired)
		{
			m_triggerFired = true;
			Common::fEventTrigger(m_pObject, 10);
			printf("[Pele] Reached desination! Mission Complete!\n");
		}

		float constexpr followDist = 10.0f;
		bool isFollow = false;
		if (!m_followData.m_finished && !m_path.empty())
		{
			Eigen::Vector3f boyPosition(-41.8087, -8.0f, -181.206);
			Eigen::Vector3f forward = m_followData.m_rotation * Eigen::Vector3f::UnitZ();
			Eigen::Vector3f diff = sonicPosition - m_followData.m_position;
			bool canSeeSonic = forward.dot(diff) > 0.0f && diff.squaredNorm() <= (followDist * followDist);
			bool canSeeBoy = (boyPosition - m_followData.m_position).squaredNorm() <= (followDist * followDist);
			isFollow = canSeeSonic || canSeeBoy;
		}

		// Handling state transition and animation looping
		float* frameTime = getFrameTime();
		float frame = *frameTime * 60.0f;
		switch (m_state)
		{
		case PeleState::PS_Wait:
		{
			if (frame > 359.0f)
			{
				*frameTime = 0.0f;
			}
			else if (isFollow)
			{
				printf("[Pele] Started following Sonic...\n");
				*frameTime = 360.0f / 60.0f;
				m_state = PeleState::PS_StandUp;
			}
			break;
		}
		case PeleState::PS_StandUp:
		{
			if (frame > 449.0f)
			{
				if (isFollow)
				{
					m_state = PeleState::PS_Follow;
				}
				else
				{
					*frameTime = 500.0f / 60.0f;
					m_state = PeleState::PS_SitDown;
				}
			}
			break;
		}
		case PeleState::PS_Follow:
		{
			if (isFollow)
			{
				if (frame > 499.0f)
				{
					*frameTime = 450.0f / 60.0f;
				}

				PathManager::followAdvance(m_followData, updateInfo.DeltaTime);
				Common::ApplyObjectPhysicsPosition(m_pObject, m_followData.m_position);
				Common::ApplyObjectPhysicsRotation(m_pObject, m_followData.m_rotation);
			}
			else
			{
				printf("[Pele] Stopped following\n");
				*frameTime = 500.0f / 60.0f;
				m_state = PeleState::PS_SitDown;
			}
			break;
		}
		case PeleState::PS_SitDown:
		{
			if (frame < 500.0f)
			{
				m_state = PeleState::PS_Wait;
			}
			break;
		}
		}
	}

	float* getFrameTime()
	{
		if (m_pObject)
		{
			uint32_t frameTimeAddress = Common::GetMultiLevelAddress((uint32_t)m_pObject + 0x110, { 0x0, 0x110, 0x4C, 0x8, 0x8 });
			if (frameTimeAddress)
			{
				return (float*)frameTimeAddress;
			}
		}

		return nullptr;
	}

	void Kill()
	{
		printf("[Pele] Killed\n");
		SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
	}
};

HOOK(void, __fastcall, Pele_MsgNotifyObjectEvent, 0xEA4F50, Sonic::CGameObject* This, void* Edx, Sonic::Message::MsgNotifyObjectEvent& message)
{
	if (message.m_Event == 401 && Common::GetCurrentStageID() == (SMT_ghz200 | SMT_Mission3))
	{
		This->m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<CObjPele>(This));
		return;
	}

	originalPele_MsgNotifyObjectEvent(This, Edx, message);
}

void Pele::applyPatches()
{
	INSTALL_HOOK(Pele_MsgNotifyObjectEvent);
}
