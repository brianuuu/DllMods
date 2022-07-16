#include "SoleannaNPC.h"
#include "Application.h"

PathDataCollection SoleannaNPC::m_pathsMapA;
PathDataCollection SoleannaNPC::m_pathsMapB;
std::map<void*, PathFollowData> SoleannaNPC::m_NPCs;
std::set<void*> SoleannaNPC::m_pObjectsMapA;
std::set<void*> SoleannaNPC::m_pObjectsMapB;
std::map<void*, float> SoleannaNPC::m_pBoxes;

HOOK(void, __fastcall, SoleannaNPC_CSonicUpdate, 0xE6BF20, void* This, void* Edx, const Hedgehog::Universe::SUpdateInfo& updateInfo)
{
	originalSoleannaNPC_CSonicUpdate(This, Edx, updateInfo);

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

	for (auto& iter : SoleannaNPC::m_NPCs)
	{
		void* pObject = iter.first;
		PathFollowData& pathFollowData = iter.second;

		if (SoleannaNPC::m_pObjectsMapA.count(pObject) && sonicPosition.z() >= -330.f && sonicPosition.x() <= 140.f
		|| SoleannaNPC::m_pObjectsMapB.count(pObject) && sonicPosition.z() >= -230.f && sonicPosition.x() >= 140.f)
		{
			if (!pathFollowData.m_enabled)
			{
				printf("[SoleannaNPC] Resuming %s following\n", pathFollowData.m_pPathData->m_name.c_str());
			}
			pathFollowData.m_enabled = true;
		}
		else 
		{
			if (pathFollowData.m_enabled)
			{
				 // Teleport out of view
				pathFollowData.m_enabled = false;
				printf("[SoleannaNPC] Paused %s following due to out of range\n", pathFollowData.m_pPathData->m_name.c_str());
				Common::ApplyObjectPhysicsPosition(pObject, Eigen::Vector3f(0, 5000, 0));
			}
			continue;
		}

		// Traverse path
		PathManager::followAdvance(pathFollowData, dt);
		Common::ApplyObjectPhysicsPosition(pObject, pathFollowData.m_position);
		Common::ApplyObjectPhysicsRotation(pObject, pathFollowData.m_rotation);
	}

	// Acclerate boxes through gravity
	for (auto& iter : SoleannaNPC::m_pBoxes)
	{
		void* pObject = iter.first;
		float& upSpeed = iter.second;

		float* pPos = (float*)(((uint32_t*)pObject)[46] + 0x70);
		Eigen::Vector3f pos(pPos[0], pPos[1], pPos[2]);

		float const minY = -7.0785f;
		float const gravity = -5.0f;

		pos.y() += upSpeed * dt + 0.5f * gravity * dt * dt;
		printf("upspeed = %.4f\n", upSpeed);
		if (pos.y() > minY)
		{
			upSpeed += gravity * dt;
			upSpeed = min(upSpeed, 40.0f);
		}
		else
		{
			pos.y() = minY;
		}
		Common::ApplyObjectPhysicsPosition(pObject, pos);
	}
}

HOOK(int*, __fastcall, SoleannaNPC_CSonicStateSquatKickBegin, 0x12526D0, void* This)
{
	if (!SoleannaNPC::m_pBoxes.empty())
	{
		Eigen::Vector3f playerPosition;
		Eigen::Quaternionf playerRotation;
		if (Common::GetPlayerTransform(playerPosition, playerRotation))
		{
			for (auto& iter : SoleannaNPC::m_pBoxes)
			{
				void* pObject = iter.first;
				float& upSpeed = iter.second;

				float* pPos = (float*)(((uint32_t*)pObject)[46] + 0x70);
				if (playerPosition.y() >= pPos[1] + 1.8f && playerPosition.y() <= pPos[1] + 2.2f &&
					playerPosition.x() >= pPos[0] - 1.0f && playerPosition.x() <= pPos[0] + 1.0f &&
					playerPosition.z() >= pPos[2] - 1.0f && playerPosition.z() <= pPos[2] + 1.0f)
				{
					upSpeed = 6.0f;
				}
			}
		}
	}

	return originalSoleannaNPC_CSonicStateSquatKickBegin(This);
}

HOOK(int, __fastcall, SoleannaNPC_CGameObject3DDestruction, 0xD5D790, void* This)
{
	SoleannaNPC::m_pObjectsMapA.erase(This);
	SoleannaNPC::m_pObjectsMapB.erase(This);
	SoleannaNPC::m_NPCs.erase(This);

	if (SoleannaNPC::m_pBoxes.count(This))
	{
		SoleannaNPC::m_pBoxes.erase(This);
		printf("[The Box] Removed object 0x%08x\n", (uint32_t)This);
	}

    return originalSoleannaNPC_CGameObject3DDestruction(This);
}

HOOK(void, __fastcall, SoleannaNPC_MsgNotifyObjectEvent, 0xEA4F50, void* This, void* Edx, uint32_t a2)
{
    uint32_t* pEvent = (uint32_t*)(a2 + 16);
    uint32_t* pObject = (uint32_t*)This;

	if (Common::CheckCurrentStage("pam000"))
	{
		if (*pEvent == 50 && !SoleannaNPC::m_pBoxes.count(This))
		{
			SoleannaNPC::m_pBoxes[This] = 0.0f;
			printf("[The Box] Added box 0x%08x\n", (uint32_t)This);
			return;
		}
		else if (*pEvent >= 101 && *pEvent <= 160)
		{
			// Event 101-120 -> ID 0-19
			// Event 121-140 -> ID 0-19 start at 1/3 of spline
			// Event 141-160 -> ID 0-19 start at 2/3 of spline
			uint32_t id = (*pEvent - 101) % 20; // 0-19
			if (id < SoleannaNPC::m_pathsMapA.size())
			{
				SoleannaNPC::m_pObjectsMapA.insert(This);

				PathFollowData data;
				data.m_yawOnly = true;
				data.m_speed = 1.4f;
				data.m_pPathData = &SoleannaNPC::m_pathsMapA[id];

				if (*pEvent > 140)
				{
					data.m_segmentID = data.m_pPathData->m_knots.size() * 2 / 3;
				}
				else if (*pEvent > 120)
				{
					data.m_segmentID = data.m_pPathData->m_knots.size() / 3;
				}

				SoleannaNPC::m_NPCs[This] = data;
				printf("[SoleannaNPC] Added object 0x%08x with mapA pathID = %u (segment %u)\n", (uint32_t)pObject, id, data.m_segmentID);
			}
			return;
		}
		else if (*pEvent >= 201 && *pEvent <= 245)
		{
			// Event 201-215 -> ID 0-14
			// Event 216-230 -> ID 0-14 start at 1/3 of spline
			// Event 231-245 -> ID 0-14 start at 2/3 of spline
			uint32_t id = (*pEvent - 201) % 15; // 0-14
			if (id < SoleannaNPC::m_pathsMapA.size())
			{
				SoleannaNPC::m_pObjectsMapB.insert(This);

				PathFollowData data;
				data.m_yawOnly = true;
				data.m_speed = 1.4f;
				data.m_pPathData = &SoleannaNPC::m_pathsMapB[id];

				if (*pEvent > 230)
				{
					data.m_segmentID = data.m_pPathData->m_knots.size() * 2 / 3;
				}
				else if (*pEvent > 215)
				{
					data.m_segmentID = data.m_pPathData->m_knots.size() / 3;
				}

				SoleannaNPC::m_NPCs[This] = data;
				printf("[SoleannaNPC] Added object 0x%08x with mapA pathID = %u (segment %u)\n", (uint32_t)pObject, id, data.m_segmentID);
			}
			return;
		}
	}

    originalSoleannaNPC_MsgNotifyObjectEvent(This, Edx, a2);
}

void SoleannaNPC::applyPatches()
{
	if (PathManager::parsePathXml(m_pathsMapA, true, "Assets\\Stage\\SoleannaNPC_mapA.path.xml") == tinyxml2::XML_SUCCESS
	 && PathManager::parsePathXml(m_pathsMapB, true, "Assets\\Stage\\SoleannaNPC_mapB.path.xml") == tinyxml2::XML_SUCCESS)
	{
		INSTALL_HOOK(SoleannaNPC_CSonicUpdate);
		INSTALL_HOOK(SoleannaNPC_CGameObject3DDestruction);
		INSTALL_HOOK(SoleannaNPC_MsgNotifyObjectEvent);
		INSTALL_HOOK(SoleannaNPC_CSonicStateSquatKickBegin);
	}
	else
	{
		MessageBox(NULL, L"Failed to parse SoleannaNPC.path.xml", NULL, MB_ICONERROR);
	}
}
