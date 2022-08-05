#include "SoleannaNPC.h"
#include "Application.h"

PathDataCollection SoleannaNPC::m_pathsMapA;
PathDataCollection SoleannaNPC::m_pathsMapB;

class CObjSoleannaNPC : public Sonic::CGameObject
{
	Sonic::CGameObject* m_pObject;
	bool m_mapA;
	PathFollowData m_followData;

public:
	CObjSoleannaNPC(Sonic::CGameObject* pObject, bool mapA, PathFollowData followData)
	{
		m_pObject = pObject;
		m_mapA = mapA;
		m_followData = followData;
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

		if (m_mapA && sonicPosition.z() >= -330.f && sonicPosition.x() <= 140.f
		|| !m_mapA && sonicPosition.z() >= -230.f && sonicPosition.x() >= 140.f)
		{
			if (!m_followData.m_enabled)
			{
				printf("[SoleannaNPC] Resuming %s following\n", m_followData.m_pPathData->m_name.c_str());
			}
			m_followData.m_enabled = true;
		}
		else
		{
			if (m_followData.m_enabled)
			{
				// Teleport out of view
				m_followData.m_enabled = false;
				printf("[SoleannaNPC] Paused %s following due to out of range\n", m_followData.m_pPathData->m_name.c_str());
				SendMessage(m_pObject->m_ActorID, boost::make_shared<Sonic::Message::MsgSetPosition>(Eigen::Vector3f(0, 5000, 0)));
			}
			return;
		}

		// Traverse path
		PathManager::followAdvance(m_followData, updateInfo.DeltaTime);
		SendMessage(m_pObject->m_ActorID, boost::make_shared<Sonic::Message::MsgSetPosition>(m_followData.m_position));
		SendMessage(m_pObject->m_ActorID, boost::make_shared<Sonic::Message::MsgSetRotation>(m_followData.m_rotation));
	}

	void Kill()
	{
		printf("[SoleannaNPC] Killed\n");
		SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
	}
};

std::map<Sonic::CGameObject*, boost::shared_ptr<Sonic::CGameObject>> m_spBoxes;
class CObjTheBox : public Sonic::CGameObject
{
	Sonic::CGameObject* m_pObject;
	float m_startYPos;
	float m_upSpeed;

	std::mutex m_mutex;

public:
	CObjTheBox(Sonic::CGameObject* pObject)
	{
		m_pObject = pObject;
		m_startYPos = (*(Eigen::Vector3f*)(((uint32_t*)pObject)[46] + 112)).y();
		m_upSpeed = 0.0f;
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
		std::lock_guard<std::mutex> guard(m_mutex);

		float* pPos = (float*)(((uint32_t*)m_pObject)[46] + 0x70);
		Eigen::Vector3f pos(pPos[0], pPos[1], pPos[2]);

		float constexpr gravity = -5.0f;
		pos.y() += m_upSpeed * updateInfo.DeltaTime + 0.5f * gravity * updateInfo.DeltaTime * updateInfo.DeltaTime;
		if (pos.y() > m_startYPos)
		{
			m_upSpeed += gravity * updateInfo.DeltaTime;
			m_upSpeed = min(m_upSpeed, 40.0f);
		}
		else
		{
			pos.y() = m_startYPos;
		}
		SendMessage(m_pObject->m_ActorID, boost::make_shared<Sonic::Message::MsgSetPosition>(pos));
	}

	void Kill()
	{
		printf("[The Box] Killed\n");
		SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
	}

	// Called outside self update so probably need mutex
	void KickUp()
	{
		Eigen::Vector3f playerPosition;
		Eigen::Quaternionf playerRotation;
		if (Common::GetPlayerTransform(playerPosition, playerRotation))
		{
			float* pPos = (float*)(((uint32_t*)m_pObject)[46] + 0x70);
			if (playerPosition.y() >= pPos[1] + 1.8f && playerPosition.y() <= pPos[1] + 2.2f &&
				playerPosition.x() >= pPos[0] - 1.0f && playerPosition.x() <= pPos[0] + 1.0f &&
				playerPosition.z() >= pPos[2] - 1.0f && playerPosition.z() <= pPos[2] + 1.0f)
			{
				std::lock_guard<std::mutex> guard(m_mutex);
				m_upSpeed = 6.0f;
			}
		}
	}
};

HOOK(int*, __fastcall, SoleannaNPC_CSonicStateSquatKickBegin, 0x12526D0, void* This)
{
	if (!m_spBoxes.empty())
	{
		for (auto& iter : m_spBoxes)
		{
			((CObjTheBox*)iter.second.get())->KickUp();
		}
	}

	return originalSoleannaNPC_CSonicStateSquatKickBegin(This);
}

HOOK(int, __fastcall, SoleannaNPC_CGameObject3DDestruction, 0xD5D790, Sonic::CGameObject* This)
{
	if (m_spBoxes.count(This))
	{
		((CObjTheBox*)m_spBoxes[This].get())->Kill();
		m_spBoxes.erase(This);
	}

	return originalSoleannaNPC_CGameObject3DDestruction(This);
}

HOOK(void, __fastcall, SoleannaNPC_MsgNotifyObjectEvent, 0xEA4F50, Sonic::CGameObject* This, void* Edx, Sonic::Message::MsgNotifyObjectEvent& message)
{
	if (message.m_Event == 50 && !m_spBoxes.count(This))
	{
		m_spBoxes[This] = boost::make_shared<CObjTheBox>(This);
		This->m_pMember->m_pGameDocument->AddGameObject(m_spBoxes[This]);
		printf("[The Box] Added box 0x%08x\n", (uint32_t)This);
		return;
	}

	if (Common::GetCurrentStageID() == SMT_pam000)
	{
		if (message.m_Event >= 101 && message.m_Event <= 160)
		{
			// Event 101-120 -> ID 0-19
			// Event 121-140 -> ID 0-19 start at 1/3 of spline
			// Event 141-160 -> ID 0-19 start at 2/3 of spline
			uint32_t id = (message.m_Event - 101) % 20; // 0-19
			if (id < SoleannaNPC::m_pathsMapA.size())
			{
				PathFollowData data;
				data.m_yawOnly = true;
				data.m_speed = 1.4f;
				data.m_pPathData = &SoleannaNPC::m_pathsMapA[id];

				if (message.m_Event > 140)
				{
					data.m_segmentID = data.m_pPathData->m_knots.size() * 2 / 3;
				}
				else if (message.m_Event > 120)
				{
					data.m_segmentID = data.m_pPathData->m_knots.size() / 3;
				}

				This->m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<CObjSoleannaNPC>(This, true, data));
				printf("[SoleannaNPC] Added object 0x%08x with mapA pathID = %u (segment %u)\n", (uint32_t)This, id, data.m_segmentID);
			}
			return;
		}
		else if (message.m_Event >= 201 && message.m_Event <= 245)
		{
			// Event 201-215 -> ID 0-14
			// Event 216-230 -> ID 0-14 start at 1/3 of spline
			// Event 231-245 -> ID 0-14 start at 2/3 of spline
			uint32_t id = (message.m_Event - 201) % 15; // 0-14
			if (id < SoleannaNPC::m_pathsMapA.size())
			{
				PathFollowData data;
				data.m_yawOnly = true;
				data.m_speed = 1.4f;
				data.m_pPathData = &SoleannaNPC::m_pathsMapB[id];

				if (message.m_Event > 230)
				{
					data.m_segmentID = data.m_pPathData->m_knots.size() * 2 / 3;
				}
				else if (message.m_Event > 215)
				{
					data.m_segmentID = data.m_pPathData->m_knots.size() / 3;
				}

				This->m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<CObjSoleannaNPC>(This, false, data));
				printf("[SoleannaNPC] Added object 0x%08x with mapA pathID = %u (segment %u)\n", (uint32_t)This, id, data.m_segmentID);
			}
			return;
		}
	}

    originalSoleannaNPC_MsgNotifyObjectEvent(This, Edx, message);
}

void SoleannaNPC::applyPatches()
{
	if (PathManager::parsePathXml(m_pathsMapA, true, "Assets\\Stage\\SoleannaNPC_mapA.path.xml") == tinyxml2::XML_SUCCESS
	 && PathManager::parsePathXml(m_pathsMapB, true, "Assets\\Stage\\SoleannaNPC_mapB.path.xml") == tinyxml2::XML_SUCCESS)
	{
		INSTALL_HOOK(SoleannaNPC_CGameObject3DDestruction);
		INSTALL_HOOK(SoleannaNPC_MsgNotifyObjectEvent);
		INSTALL_HOOK(SoleannaNPC_CSonicStateSquatKickBegin);
	}
	else
	{
		MessageBox(NULL, L"Failed to parse SoleannaNPC.path.xml", NULL, MB_ICONERROR);
	}
}
