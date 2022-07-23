#include "SoleannaBoys.h"

PathDataCollection SoleannaBoys::m_paths;
std::map<void*, SoleannaBoys::BoyData> SoleannaBoys::m_boys;
std::deque<void*> SoleannaBoys::m_omochaos;

HOOK(int, __fastcall, SoleannaBoys_OmochaoMsgNotifyObjectEvent, 0x114FB60, void* This, void* Edx, uint32_t a2)
{
	uint32_t* pEvent = (uint32_t*)(a2 + 16);

	if (Common::GetCurrentStageID() == (SMT_ghz200 | SMT_Mission4))
	{
		if (*pEvent >= 421 && *pEvent <= 425)
		{
			if (SoleannaBoys::m_omochaos.empty())
			{
				SoleannaBoys::m_omochaos.resize(5);
			}

			int index = *pEvent - 421;
			SoleannaBoys::m_omochaos[index] = This;
			printf("[SoleannaBoys] Omochao added with address 0x%08x\n", (uint32_t)This);

			return 0;
		}
	}

	return originalSoleannaBoys_OmochaoMsgNotifyObjectEvent(This, Edx, a2);
}

HOOK(void, __fastcall, SoleannaBoys_MsgNotifyObjectEvent, 0xEA4F50, void* This, void* Edx, uint32_t a2)
{
	uint32_t* pEvent = (uint32_t*)(a2 + 16);
	uint32_t* pObject = (uint32_t*)This;

	if (Common::GetCurrentStageID() == (SMT_ghz200 | SMT_Mission4))
	{
		if (SoleannaBoys::m_paths.empty())
		{
			if (!PathManager::parsePathXml(SoleannaBoys::m_paths, true, "Assets\\Stage\\SoleannaBoysChallenge.path.xml") == tinyxml2::XML_SUCCESS)
			{
				MessageBox(NULL, L"Failed to parse SoleannaBoysChallenge.path.xml", NULL, MB_ICONERROR);
			}
		}

		if (*pEvent >= 411 && *pEvent <= 416)
		{
			int index = *pEvent - 411;
			SoleannaBoys::BoyData& data = SoleannaBoys::m_boys[This];
			data.m_pObjectRunning = This;
			data.m_stopped = false;

			data.m_followData = PathFollowData();
			data.m_followData.m_yawOnly = true;
			data.m_followData.m_speed = 4.0f;
			data.m_followData.m_pPathData = &SoleannaBoys::m_paths[index];

			printf("[SoleannaBoys] Boy added with address 0x%08x\n", (uint32_t)This);
			return;
		}
	}

	originalSoleannaBoys_MsgNotifyObjectEvent(This, Edx, a2);
}

HOOK(void, __fastcall, SoleannaBoys_CSonicUpdate, 0xE6BF20, void* This, void* Edx, const Hedgehog::Universe::SUpdateInfo& updateInfo)
{
	originalSoleannaBoys_CSonicUpdate(This, Edx, updateInfo);

	if (SoleannaBoys::m_paths.empty() || SoleannaBoys::m_boys.empty()) return;

	Eigen::Vector3f sonicPosition;
	Eigen::Quaternionf sonicRotation;
	if (!Common::GetPlayerTransform(sonicPosition, sonicRotation))
	{
		return;
	}

	// Account for object slow time from red gem
	float dt = updateInfo.DeltaTime;
	if (*(bool*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x19D }))
	{
		dt *= *(float*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x1A4 });
	}

	for (auto& iter : SoleannaBoys::m_boys)
	{
		SoleannaBoys::BoyData& data = iter.second;
		if (!data.m_stopped)
		{
			float constexpr hitDist = 0.5f;
			if ((data.m_followData.m_position - sonicPosition).squaredNorm() <= (hitDist * hitDist))
			{
				data.m_stopped = true;

				static SharedPtrTypeless soundHandle;
				Common::PlaySoundStatic(soundHandle, 845019953);
				FUNCTION_PTR(void*, __stdcall, SoleannaBoys_fpEventTrigger, 0xD5ED00, void* This, int Event);
				SoleannaBoys_fpEventTrigger(data.m_pObjectRunning, 10);

				if (!SoleannaBoys::m_omochaos.empty())
				{
					struct MsgNotifyObjectEvent
					{
						INSERT_PADDING(0x10);
						uint32_t m_event;
						bool m_unknown;
					};
					alignas(16) MsgNotifyObjectEvent msgNotifyObjectEvent {};
					msgNotifyObjectEvent.m_event = 6;
					msgNotifyObjectEvent.m_unknown = false;
					originalSoleannaBoys_OmochaoMsgNotifyObjectEvent(SoleannaBoys::m_omochaos.front(), Edx, (uint32_t)&msgNotifyObjectEvent);
					SoleannaBoys::m_omochaos.pop_front();
				}
			}
			else
			{
				PathManager::followAdvance(data.m_followData, dt);
				Common::ApplyObjectPhysicsPosition(data.m_pObjectRunning, data.m_followData.m_position);
				Common::ApplyObjectPhysicsRotation(data.m_pObjectRunning, data.m_followData.m_rotation);
			}
		}

		// Handle animation
		float* frameTime = (float*)Common::GetMultiLevelAddress((uint32_t)data.m_pObjectRunning + 0x110, { 0x0, 0x110, 0x4C, 0x8, 0x8 });
		float frame = *frameTime * 60.0f;
		if (data.m_stopped)
		{
			if (frame <= 36.0f || frame >= 163.0f)
			{
				*frameTime = 38.0f / 60.0f;
			}
		}
		else
		{
			if (frame > 35.0f)
			{
				*frameTime = 0.0f;
			}
		}
	}
}

HOOK(int, __fastcall, SoleannaBoys_CGameObject3DDestruction, 0xD5D790, void* This)
{
	SoleannaBoys::m_boys.erase(This);
	return originalSoleannaBoys_CGameObject3DDestruction(This);
}

HOOK(int, __fastcall, SoleannaBoys_MsgRestartStage, 0xE76810, uint32_t* This, void* Edx, void* message)
{
	SoleannaBoys::m_omochaos.clear();
	return originalSoleannaBoys_MsgRestartStage(This, Edx, message);
}

void SoleannaBoys::applyPatches()
{
	INSTALL_HOOK(SoleannaBoys_MsgNotifyObjectEvent);
	INSTALL_HOOK(SoleannaBoys_OmochaoMsgNotifyObjectEvent);
	INSTALL_HOOK(SoleannaBoys_CSonicUpdate);
	INSTALL_HOOK(SoleannaBoys_CGameObject3DDestruction);
	INSTALL_HOOK(SoleannaBoys_MsgRestartStage);
}
