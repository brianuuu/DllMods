#include "Pele.h"

Pele::PeleData Pele::m_data;

HOOK(void, __fastcall, Pele_MsgNotifyObjectEvent, 0xEA4F50, void* This, void* Edx, uint32_t a2)
{
	uint32_t* pEvent = (uint32_t*)(a2 + 16);
	uint32_t* pObject = (uint32_t*)This;

	if (*pEvent == 401 && Common::GetCurrentStageID() == (SMT_ghz200 | SMT_Mission3))
	{
		Pele::m_data.parsePath();
		Pele::m_data.reset();
		Pele::m_data.m_pObject = This;
		printf("[Pele] Pele the Beloved Dog address 0x%08x\n", (uint32_t)This);

		return;
	}

	originalPele_MsgNotifyObjectEvent(This, Edx, a2);
}

HOOK(int, __fastcall, Pele_CGameObject3DDestruction, 0xD5D790, void* This)
{
	if (Pele::m_data.m_pObject == This)
	{
		Pele::m_data.reset();
		printf("[Pele] Removed object 0x%08x\n", (uint32_t)This);
	}

	return originalPele_CGameObject3DDestruction(This);
}

HOOK(void, __fastcall, Pele_CSonicUpdate, 0xE6BF20, void* This, void* Edx, const Hedgehog::Universe::SUpdateInfo& updateInfo)
{
	originalPele_CSonicUpdate(This, Edx, updateInfo);

	// Account for object slow time from red gem
	float dt = updateInfo.DeltaTime;
	if (*(bool*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x19D }))
	{
		dt *= *(float*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x1A4 });
	}
	Pele::m_data.advance(dt);
}

void Pele::applyPatches()
{
	INSTALL_HOOK(Pele_MsgNotifyObjectEvent);
	INSTALL_HOOK(Pele_CGameObject3DDestruction);
	INSTALL_HOOK(Pele_CSonicUpdate);
}

void Pele::PeleData::parsePath()
{
	if (m_path.empty())
	{
		if (!PathManager::parsePathXml(m_path, false, "Assets\\Stage\\Pele.path.xml") == tinyxml2::XML_SUCCESS)
		{
			MessageBox(NULL, L"Failed to parse Pele.path.xml", NULL, MB_ICONERROR);
		}
	}
}

void Pele::PeleData::reset()
{
	m_followData = PathFollowData();
	if (!m_path.empty())
	{
		m_followData.m_yawOnly = true;
		m_followData.m_speed = 4.0f;
		m_followData.m_loop = false;
		m_followData.m_pPathData = &m_path[0];

		m_followData.m_rotation = Eigen::AngleAxisf(PI_F / 2.0f, Eigen::Vector3f::UnitY()) * Eigen::Quaternionf::Identity();
		m_followData.m_position = m_path[0].m_knots[0].m_point;
	}

	m_pObject = nullptr;
	m_state = PeleState::PS_Wait;
	m_triggerFired = false;
}

void Pele::PeleData::advance(float dt)
{
	Eigen::Vector3f sonicPosition;
	Eigen::Quaternionf sonicRotation;
	if (m_pObject == nullptr || !Common::GetPlayerTransform(sonicPosition, sonicRotation))
	{
		return;
	}

	if (m_followData.m_finished && !m_triggerFired)
	{
		m_triggerFired = true;
		FUNCTION_PTR(void*, __stdcall, Pele_fpEventTrigger, 0xD5ED00, void* This, int Event);
		Pele_fpEventTrigger(m_pObject, 10);
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

			PathManager::followAdvance(m_followData, dt);
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

float* Pele::PeleData::getFrameTime()
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
