#include "Itembox.h"

float const c_10ringRadius = 0.57f;
float const c_1upRadius = 0.70f;

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
	switch (This[71])
	{
	default: WRITE_MEMORY(0xFFF82F, uint32_t, 2) break; // 1up
	case 1:  WRITE_MEMORY(0xFFF82F, uint32_t, 4) break; // invincibility
	case 2:  WRITE_MEMORY(0xFFF82F, uint32_t, 0) break; // shield
	case 3:  WRITE_MEMORY(0xFFF82F, uint32_t, 8) break; // fire shield
	case 4:  WRITE_MEMORY(0xFFF82F, uint32_t, 3) break; // speed shoes
	case 5:  WRITE_MEMORY(0xFFF82F, uint32_t, 20) break; // gauge up
	case 6:  WRITE_MEMORY(0xFFF82F, uint32_t, 21) break; // 5 ring
	case 7:  WRITE_MEMORY(0xFFF82F, uint32_t, 22) break; // 10 ring
	case 8:  WRITE_MEMORY(0xFFF82F, uint32_t, 23) break; // 20 ring
	}

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
		float const maxBoost = (Common::GetPlayerSkill() & 0x8) ? 200.0f : 100.0f;
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

const char* volatile const ObjectProductionItemboxLock = "ObjectProductionItemboxLock.phy.xml";
uint32_t LoadItemboxLockAsmHookReturnAddress = 0xD45FAA;
uint32_t sub_EA0450 = 0xEA0450;
void __declspec(naked) LoadItemboxLockAsmHook()
{
	__asm
	{
		call	[CStringDestructor]
		push    ObjectProductionItemboxLock
		lea     ecx, [esp + 0x10]
		call	[CStringConstructor]
		lea     ecx, [esp + 0xC]
		push    ecx
		push    esi
		call	[sub_EA0450]
		lea     ecx, [esp + 0xC]
		call	[CStringDestructor]
		jmp		[LoadItemboxLockAsmHookReturnAddress]
	}
}

// Have to use ASM to prevent double playing sfx
uint32_t sub_D5D940 = 0xD5D940;
uint32_t objItemPlaySfxReturnAddress = 0xFFF9A8;
void __declspec(naked) objItemPlaySfx()
{
	__asm
	{
		mov     ecx, [esp + 14h]
		call	Itembox::playItemboxPfx
		call	Itembox::playItemboxSfx
		mov     esi, [esp + 14h]
		call	[sub_D5D940]
		jmp		[objItemPlaySfxReturnAddress]
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
	WRITE_JUMP(0xD45FA5, LoadItemboxLockAsmHook);

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

	// Force all item to use cmn_obj_oneup_HD skl and anm
	WRITE_NOP(0xFFF414, 6);
	WRITE_NOP(0xFFF593, 6);
	WRITE_MEMORY(0xFFF5D6, uint8_t, 0xB8, 0x3C, 0x46, 0x66, 0x01, 0x90, 0x90);
	WRITE_MEMORY(0xFFF62A, uint8_t, 0xB8, 0x3C, 0x46, 0x66, 0x01, 0x90, 0x90);
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
				printf("[Itembox] cmn_itembox_lock (%u) already injected!\n", newSetObjectID);
				continue;
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
