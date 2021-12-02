#include "Itembox.h"
#include "UIContext.h"
#include "Application.h"
#include "Configuration.h"

float const c_10ringRadius = 0.57f;
float const c_1upRadius = 0.70f;

std::deque<ItemboxGUI> Itembox::m_guiData;
PDIRECT3DTEXTURE9 Itembox::m_item_5ring = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_10ring = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_20ring = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_1up = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_invin = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_speed = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_gauge = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_shield = nullptr;
PDIRECT3DTEXTURE9 Itembox::m_item_fire = nullptr;

std::string m_setdataLayer;
HOOK(bool, __stdcall, ParseSetdata, 0xEB5050, void* a1, char** pFileName, void* a3, void* a4, uint8_t a5, uint8_t a6)
{
	m_setdataLayer = std::string(*(char**)pFileName);
	bool result = originalParseSetdata(a1, pFileName, a3, a4, a5, a6);
	m_setdataLayer.clear();

	return result;
}

HOOK(uint32_t*, __fastcall, ReadXmlData, 0xCE5FC0, uint32_t size, char* pData, void* a3, uint32_t* a4)
{
    // Get all 1up and 10ring objects, add cmn_itembox_lock at the same position
    std::string injectStr;
	if (!m_setdataLayer.empty())
	{
		if (Itembox::getInjectStr(pData, size, injectStr) == tinyxml2::XML_SUCCESS && !injectStr.empty())
		{
			const size_t newSize = size + injectStr.size();
			const std::unique_ptr<char[]> pBuffer = std::make_unique<char[]>(newSize);
			memcpy(pBuffer.get(), pData, size);

			char* pInsertionPos = strstr(pBuffer.get(), "</SetObject>");

			memmove(pInsertionPos + injectStr.size(), pInsertionPos, size - (size_t)(pInsertionPos - pBuffer.get()));
			memcpy(pInsertionPos, injectStr.c_str(), injectStr.size());

			return originalReadXmlData(newSize, pBuffer.get(), a3, a4);
		}
	}
    
	return originalReadXmlData(size, pData, a3, a4);
}

HOOK(void, __fastcall, ItemMsgHitEventCollision, 0xFFF810, uint32_t* This, void* Edx, void* message)
{
	uint32_t type = 0;
	ItemboxType enumType = IT_COUNT;

	switch (This[71])
	{
	default:	type = 2;	enumType = IT_1up;		break; // 1up
	case 1:		type = 4;	enumType = IT_invin;	break; // invincibility
	case 2:		type = 0;	enumType = IT_shield;	break; // shield
	case 3:		type = 8;	enumType = IT_fire;		break; // fire shield
	case 4:		type = 3;	enumType = IT_speed;	break; // speed shoes
	case 5:		type = 20;	enumType = IT_gauge;	break; // gauge up
	case 6:		type = 21;	enumType = IT_5ring;	break; // 5 ring
	case 7:		type = 22;	enumType = IT_10ring;	break; // 10 ring
	case 8:		type = 23;	enumType = IT_20ring;	break; // 20 ring
	}

	// Modify what passes to MsgGetItemType
	WRITE_MEMORY(0xFFF82F, uint32_t, type);

	// Draw ImGui
	Itembox::addItemToGui(enumType);

	originalItemMsgHitEventCollision(This, Edx, message);
}

struct MsgTakeObject
{
	INSERT_PADDING(0x10);
	uint32_t m_type;
};

HOOK(void, __fastcall, Itembox_MsgTakeObject, 0xE6DEC0, void* This, void* Edx, MsgTakeObject* message)
{
	// Set CPlayerSpeedStatePluginRingCountUp amount
	switch (message->m_type)
	{
	case 21: WRITE_MEMORY(0xE40DDF, uint32_t, 5); break;
	default: WRITE_MEMORY(0xE40DDF, uint32_t, 10); break;
	case 23: WRITE_MEMORY(0xE40DDF, uint32_t, 20); break;
	}

	if (message->m_type > 20)
	{
		message->m_type = 1;
	}

	originalItembox_MsgTakeObject(This, Edx, message);
}

HOOK(void, __fastcall, Itembox_MsgGetItemType, 0xE6D7D0, void* This, void* Edx, void* a2)
{
	uint32_t type = *(uint32_t*)((uint32_t)a2 + 16);
	if (type == 20)
	{
		// Gauge up
		float const maxBoost = Common::GetPlayerMaxBoost();
		float* currentBoost = Common::GetPlayerBoost();
		if (*currentBoost < maxBoost)
		{
			*currentBoost = maxBoost;
		}
	}
	else if (type >= 21 && type <= 23)
	{
		// 5,10,20 ring
		MsgTakeObject msgTakeObject = {};
		msgTakeObject.m_type = type;

		FUNCTION_PTR(void, __thiscall, processMsgTakeObject, 0xE6DEC0, void* This, void* pMessage);
		void* player = *(void**)((uint32_t)*PLAYER_CONTEXT + 0x110);
		processMsgTakeObject(player, &msgTakeObject);
	}

	originalItembox_MsgGetItemType(This, Edx, a2);
}

HOOK(void, __fastcall, Itembox_GetSuperRing, 0x11F2F10, uint32_t* This, void* Edx, void* message)
{
	Itembox::addItemToGui(ItemboxType::IT_10ring);
	originalItembox_GetSuperRing(This, Edx, message);
}

const char* volatile const ObjectProductionItemboxLock = "ObjectProductionItemboxLock.phy.xml";
HOOK(void, __stdcall, Itembox_LoadObjectProduction, 0xEA0450, void* a1, Hedgehog::Base::CSharedString* pName)
{
	if (strstr(pName->m_pStr, "ObjectProduction.phy.xml"))
	{
		printf("[Itembox] Injecting %s\n", ObjectProductionItemboxLock);
		static Hedgehog::Base::CSharedString pInjectName(ObjectProductionItemboxLock);
		originalItembox_LoadObjectProduction(a1, &pInjectName);
	}

	originalItembox_LoadObjectProduction(a1, pName);
}

// Have to use ASM to prevent double playing sfx
void __declspec(naked) objItemPlaySfx()
{
	static uint32_t sub_D5D940 = 0xD5D940;
	static uint32_t returnAddress = 0xFFF9A8;
	__asm
	{
		mov     ecx, [esp + 14h]
		call	Itembox::playItemboxPfx
		call	Itembox::playItemboxSfx
		mov     esi, [esp + 14h]
		call	[sub_D5D940]
		jmp		[returnAddress]
	}
}

void Itembox::applyPatches()
{
	// Play itembox sfx for 1up and 10ring
	WRITE_MEMORY(0x011F2FE0, uint32_t, 4002032);
	WRITE_JUMP(0xFFF99F, objItemPlaySfx);

	// Set itembox radius
	WRITE_MEMORY(0x11F3353, float*, &c_10ringRadius);
	WRITE_MEMORY(0xFFF9E8, float*, &c_1upRadius);

	// Disable ef_ch_sns_yh1_ringget on 10ring
	WRITE_STRING(0x166E4CC, "");

	// Disable ef_ch_sng_yh1_1up after getting 1up
	WRITE_STRING(0x15E90DC, "");

	// Disable ef_ob_com_yh1_1up effect on 1up
	WRITE_JUMP(0xFFFA5E, (void*)0xFFFB15);

	// Load itembox lock-on object physics
	INSTALL_HOOK(Itembox_LoadObjectProduction);

	// Disable Item always facing screen
	WRITE_NOP(0xFFF4E3, 3);

	// Inject lock-on object
	INSTALL_HOOK(ParseSetdata);
	INSTALL_HOOK(ReadXmlData);

	//---------------------------------------
	// Inject new types to Item
	//---------------------------------------
	// Changes item model and effect
	static char const* itemNames[] = 
	{ 
		"cmn_obj_oneup_HD", 
		"cmn_obj_muteki_HD", // invincibility
		"cmn_obj_shield_HD",
		"cmn_obj_baria_HD",  // fire shield
		"cmn_obj_speed_HD",
		"cmn_obj_gaugeup_HD",
		"cmn_obj_5ring_HD",
		"cmn_obj_10ring_HD",
		"cmn_obj_20ring_HD"
	};
	WRITE_MEMORY(0xFFF566, char**, itemNames);
	INSTALL_HOOK(ItemMsgHitEventCollision);
	INSTALL_HOOK(Itembox_MsgGetItemType);
	INSTALL_HOOK(Itembox_MsgTakeObject);

	// Make CPlayerSpeedStatePluginRingCountUp get all rings immediately
	WRITE_NOP(0xE4155E, 2);
	WRITE_NOP(0xE415C8, 2);
	WRITE_MEMORY(0xE4157A, int, -1);

	// Force all item to use cmn_obj_oneup_HD skl and anm
	WRITE_NOP(0xFFF414, 6);
	WRITE_NOP(0xFFF593, 6);
	WRITE_MEMORY(0xFFF5D6, uint8_t, 0xB8, 0x3C, 0x46, 0x66, 0x01, 0x90, 0x90);
	WRITE_MEMORY(0xFFF62A, uint8_t, 0xB8, 0x3C, 0x46, 0x66, 0x01, 0x90, 0x90);

	// Prevent Disable Lives code conflict
	WRITE_MEMORY(0xFFF5D7, char*, itemNames[0]);
	WRITE_MEMORY(0xFFF62B, char*, itemNames[0]);

	// Draw GUI
	INSTALL_HOOK(Itembox_GetSuperRing);
}

void Itembox::playItemboxSfx()
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlaySound(soundHandle, 4002032, 0);
}

void __fastcall Itembox::playItemboxPfx(void* This)
{
	Common::ObjectCGlitterPlayerOneShot(This, "ef_ch_sns_yh1_1upget_s");
}

tinyxml2::XMLError Itembox::getInjectStr(char const* pData, uint32_t size, std::string& injectStr)
{
	injectStr.clear();

	tinyxml2::XMLDocument pathXml;
	tinyxml2::XMLError result = pathXml.Parse(pData, size);
	XMLCheckResult(result);

	tinyxml2::XMLNode* pRoot = pathXml.FirstChildElement("SetObject");
	if (pRoot == nullptr) return tinyxml2::XML_ERROR_FILE_READ_ERROR;

	struct PositionStr
	{
		std::string x, y, z;
	};
	typedef std::map<uint32_t, PositionStr> ItemboxData;

	std::set<uint32_t> setObjectIDs;
	ItemboxData itemboxData;
	for (tinyxml2::XMLElement* pObjectElement = pRoot->FirstChildElement(); pObjectElement != nullptr; pObjectElement = pObjectElement->NextSiblingElement())
	{
		// Grab all the object ID to check if we have already injected this code
		tinyxml2::XMLElement* pSetObjectIDElement = pObjectElement->FirstChildElement("SetObjectID");
		if (pSetObjectIDElement)
		{
			char const* setObjectIDChar = pSetObjectIDElement->GetText();
			if (!setObjectIDChar) continue;

			uint32_t setObjectID = std::stoul(setObjectIDChar);
			setObjectIDs.insert(setObjectID);

			std::string objName = pObjectElement->Name();
			if (objName != "Item" && objName != "SuperRing") continue;

			PositionStr positionStr;
			tinyxml2::XMLElement* pPositionElement = pObjectElement->FirstChildElement("Position");
			positionStr.x = pPositionElement->FirstChildElement("x")->GetText();
			positionStr.y = pPositionElement->FirstChildElement("y")->GetText();
			positionStr.z = pPositionElement->FirstChildElement("z")->GetText();

			itemboxData[setObjectID] = positionStr;
		}
	}

	if (!itemboxData.empty())
	{
		printf("[Itembox] Current layer: %s\n", m_setdataLayer.c_str());
		for (auto const& iter : itemboxData)
		{
			uint32_t const newSetObjectID = iter.first * 100;
			PositionStr const& positionStr = iter.second;

			// Only inject if ID not exist already
			if (setObjectIDs.find(newSetObjectID) != setObjectIDs.end())
			{
				printf("[Itembox] WARNING: cmn_itembox_lock already injected!\n");
				return tinyxml2::XML_ERROR_COUNT;
			}

			printf("[Itembox] Injecting cmn_itembox_lock (%u) at pos: %s, %s, %s\n", newSetObjectID, positionStr.x.c_str(), positionStr.y.c_str(), positionStr.z.c_str());
			injectStr += "<ObjectPhysics>\n";
			injectStr += "    <AddRange>0</AddRange>\n";
			injectStr += "    <CullingRange>15</CullingRange>\n";
			injectStr += "    <DebrisTarget>\n";
			injectStr += "        <x>0</x>\n";
			injectStr += "        <y>0</y>\n";
			injectStr += "        <z>0</z>\n";
			injectStr += "    </DebrisTarget>\n";
			injectStr += "    <GroundOffset>0</GroundOffset>\n";
			injectStr += "    <IsCastShadow>true</IsCastShadow>\n";
			injectStr += "    <IsDynamic>false</IsDynamic>\n";
			injectStr += "    <IsReset>false</IsReset>\n";
			injectStr += "    <Range>100</Range>\n";
			injectStr += "    <SetObjectID>" + std::to_string(newSetObjectID) + "</SetObjectID>\n";
			injectStr += "    <Type>cmn_itembox_lock</Type>\n";
			injectStr += "    <WrappedObjectID>\n";
			injectStr += "        <SetObjectID>0</SetObjectID>\n";
			injectStr += "    </WrappedObjectID>\n";
			injectStr += "    <Position>\n";
			injectStr += "        <x>" + positionStr.x + "</x>\n";
			injectStr += "        <y>" + positionStr.y + "</y>\n";
			injectStr += "        <z>" + positionStr.z + "</z>\n";
			injectStr += "    </Position>\n";
			injectStr += "    <Rotation>\n";
			injectStr += "        <x>0</x>";
			injectStr += "        <y>0</y>";
			injectStr += "        <z>0</z>";
			injectStr += "        <w>1</w>";
			injectStr += "    </Rotation>\n";
			injectStr += "</ObjectPhysics>\n";
		}
	}

	return tinyxml2::XML_SUCCESS;
}

void Itembox::addItemToGui(ItemboxType type)
{
	if (type >= ItemboxType::IT_COUNT) return;

	// Initialize GUI data
	m_guiData.push_front(ItemboxGUI(type));
	
	// Only allow 5 to display at max
	if (m_guiData.size() > 5)
	{
		m_guiData.pop_back();
	}
}

bool Itembox::initTextures()
{
	std::wstring const dir = Application::getModDirWString();
	bool success = true;
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_5ring.dds").c_str(), &m_item_5ring);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_10ring.dds").c_str(), &m_item_10ring);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_20ring.dds").c_str(), &m_item_20ring);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_1up.dds").c_str(), &m_item_1up);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_invin.dds").c_str(), &m_item_invin);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_speed.dds").c_str(), &m_item_speed);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_gauge.dds").c_str(), &m_item_gauge);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_shield.dds").c_str(), &m_item_shield);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_fire.dds").c_str(), &m_item_fire);

	if (!success)
	{
		MessageBox(nullptr, TEXT("Failed to load assets for itembox icons, they may not display correctly"), TEXT("STH2006 Project"), MB_ICONWARNING);
	}

	return success;
}

void Itembox::draw()
{
	// At loading screen, clear all
	if (Common::IsAtLoadingScreen())
	{
		m_guiData.clear();
		return;
	}

	// Remove frames that has timed out
	while (!m_guiData.empty() && m_guiData.back().m_frame > 200.0f)
	{
		m_guiData.pop_back();
	}

	// 0-8: zoom in
	// 9: zoom to size
	// 10-200: stay
	for (int i = m_guiData.size() - 1; i >= 0; i--)
	{
		ItemboxGUI& data = m_guiData[i];

		PDIRECT3DTEXTURE9 texture = nullptr;
		switch (data.m_type)
		{
		case IT_5ring:	 texture = m_item_5ring;  break;
		case IT_10ring:	 texture = m_item_10ring; break;
		case IT_20ring:	 texture = m_item_20ring; break;
		case IT_1up:	 texture = m_item_1up;	  break;
		case IT_invin:	 texture = m_item_invin;  break;
		case IT_speed:	 texture = m_item_speed;  break;
		case IT_gauge:	 texture = m_item_gauge;  break;
		case IT_shield:	 texture = m_item_shield; break;
		case IT_fire:	 texture = m_item_fire;	  break;
		}

		if (data.m_frame > 0.0f && texture)
		{
			static bool visible = true;
			ImGui::Begin((std::string("Itembox") + std::to_string(i)).c_str(), &visible, UIContext::m_hudFlags);
			{
				float sizeX = *BACKBUFFER_WIDTH * 192.0f / 1280.0f;
				float sizeY = *BACKBUFFER_HEIGHT * 192.0f / 720.0f;
				if (data.m_frame <= 8.0f)
				{
					float scale = data.m_frame / 8.0f * 1.12f;
					sizeX *= scale;
					sizeY *= scale;
				}
				else if (data.m_frame < 10.0f)
				{
					float scale = ((10.0f - data.m_frame) * 0.12f) + 1.0f;
					sizeX *= scale;
					sizeY *= scale;
				}

				float targetPos = 0.5f - (0.091f * i) * (Configuration::m_using06HUD ? 1.0f : -1.0f);
				data.m_pos += (targetPos - data.m_pos) * 0.15f * Application::getHudDeltaTime() * 60.0f;
				float posX = floorf(*BACKBUFFER_WIDTH * data.m_pos - sizeX * 0.5f - 8.0f);
				float posY = floorf(*BACKBUFFER_HEIGHT * 0.847f - sizeX * 0.5f - 8.0f);
				ImGui::SetWindowFocus();
				ImGui::SetWindowPos(ImVec2(posX, posY));
				ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
				ImGui::Image(texture, ImVec2(sizeX, sizeY));
			}
			ImGui::End();
		}

		data.m_frame += Application::getHudDeltaTime() * 60.0f;
	}
}
