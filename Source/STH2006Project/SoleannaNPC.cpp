#include "SoleannaNPC.h"

PathDataCollection SoleannaNPC::m_pathsMapA;
PathDataCollection SoleannaNPC::m_pathsMapB;
std::map<void*, PathFollowData> SoleannaNPC::m_NPCs;
std::set<void*> SoleannaNPC::m_pObjectsMapA;
std::set<void*> SoleannaNPC::m_pObjectsMapB;

HOOK(void, __fastcall, SoleannaNPC_CSonicUpdate, 0xE6BF20, void* This, void* Edx, float* dt)
{
	originalSoleannaNPC_CSonicUpdate(This, Edx, dt);

	Eigen::Vector3f sonicPosition;
	Eigen::Quaternionf sonicRotation;
	if (!Common::GetPlayerTransform(sonicPosition, sonicRotation))
	{
		return;
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
				Common::applyObjectPhysicsPosition(pObject, Eigen::Vector3f(0, 5000, 0));
			}
			continue;
		}

		// Traverse oath
		PathManager::followAdvance(pathFollowData, *dt);
		Common::applyObjectPhysicsPosition(pObject, pathFollowData.m_position);
		Common::applyObjectPhysicsRotation(pObject, pathFollowData.m_rotation);
	}
}

HOOK(int, __fastcall, SoleannaNPC_CGameObject3DDestruction, 0xD5D790, void* This)
{
	SoleannaNPC::m_pObjectsMapA.erase(This);
	SoleannaNPC::m_pObjectsMapB.erase(This);
	SoleannaNPC::m_NPCs.erase(This);

    return originalSoleannaNPC_CGameObject3DDestruction(This);
}

HOOK(void, __fastcall, SoleannaNPC_MsgNotifyObjectEvent, 0xEA4F50, void* This, void* Edx, uint32_t a2)
{
    uint32_t* pEvent = (uint32_t*)(a2 + 16);
    uint32_t* pObject = (uint32_t*)This;

	if (Common::CheckCurrentStage("pam000"))
	{
		if (*pEvent >= 101 && *pEvent <= 160)
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
		}
	}

    originalSoleannaNPC_MsgNotifyObjectEvent(This, Edx, a2);
}

void SoleannaNPC::applyPatches()
{
	if (PathManager::parsePathXml(m_pathsMapA, true, "SoleannaNPC_mapA.path.xml") == tinyxml2::XML_SUCCESS
	 && PathManager::parsePathXml(m_pathsMapB, true, "SoleannaNPC_mapB.path.xml") == tinyxml2::XML_SUCCESS)
	{
		INSTALL_HOOK(SoleannaNPC_CSonicUpdate);
		INSTALL_HOOK(SoleannaNPC_CGameObject3DDestruction);
		INSTALL_HOOK(SoleannaNPC_MsgNotifyObjectEvent);
	}
	else
	{
		MessageBox(NULL, L"Failed to parse SoleannaNPC.path.xml", NULL, MB_ICONERROR);
	}
}
