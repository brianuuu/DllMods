#include "Chip.h"
#include "Configuration.h"

#define PI 3.14159265

void** const PLAYER_CONTEXT = (void**)0x1E5E2F0;
void** const pModernSonicContext = (void**)0x1E5E2F8;
void** const pClassicSonicContext = (void**)0x1E5E304;
Chip::ChipResult Chip::m_chipResult;
Chip::ChipFollow Chip::m_chipFollow;
Chip::ChipEye* Chip::m_chipEyeL;
Chip::ChipEye* Chip::m_chipEyeR;
std::vector<float> Chip::m_chipEyeEndTimes = {1, 7, 12, 16, 22, 26, 31, 37, 42, 46, 51, 54, 59};

uint32_t const sub_EA0450 = 0xEA0450;
uint32_t const CStringConstructor = 0x6621A0;
uint32_t const CStringDestructor = 0x661550;

const char* volatile const ObjectProductionChip = "ObjectProductionChip.phy.xml";
HOOK(void, __stdcall, LoadObjectProduction, 0xEA0450, void* a1, Hedgehog::Base::CSharedString* pName)
{
    if (strstr(pName->m_pStr, "ObjectProduction.phy.xml"))
    {
        printf("[Itembox] Injecting %s\n", ObjectProductionChip);
        static Hedgehog::Base::CSharedString pInjectName(ObjectProductionChip);
        originalLoadObjectProduction(a1, &pInjectName);
    }

    originalLoadObjectProduction(a1, pName);
}

bool getAtHubWorld()
{
    std::string const currentStageID((char*)0x01E774D4, 3);
    return currentStageID == "pam";
}

StartMode Chip::m_startMode = StartMode::INVALID;
Eigen::Vector3f Chip::m_sonicStartPos(0, 0, 0);
HOOK(uint32_t, __stdcall, ParseStageStgXml, 0x11D27A0, void* a1, void* a2, char* pData, const size_t size)
{
    // Loading new stage, reset eye pointer here
    printf("[Chip Eye] Resetting Chip eye pointers\n");
    Chip::m_chipEyeL = nullptr;
    Chip::m_chipEyeR = nullptr;

    std::string str(pData, size);

    // Get starting mode
    if (str.find("<Mode>Stand</Mode>") != std::string::npos)
    {
        Chip::m_startMode = StartMode::Stand;
        DebugDrawText::log("Start Mode = Stand");
    }
    else if (str.find("<Mode>CrouchingStartFront</Mode>") != std::string::npos)
    {
        Chip::m_startMode = StartMode::CrouchingStartFront;
        DebugDrawText::log("Start Mode = CrouchingStartFront");
    }
    else
    {
        Chip::m_startMode = StartMode::INVALID;
        DebugDrawText::log("Start Mode = Others");
    }

    /* Get Sonic's yaw
    size_t yawStart = str.find("<Yaw>");
    size_t yawEnd = str.find("</Yaw>");
    if (yawStart != std::string::npos && yawEnd != std::string::npos)
    {
        std::string yawStr = str.substr(yawStart + 5, yawEnd - yawStart - 5);
        Chip::m_sonicYaw = std::stof(yawStr);
        DebugDrawText::log(format("Sonic's Yaw = %.3f", Chip::m_sonicYaw));
    }
    else
    {
        Chip::m_startMode = StartMode::INVALID;
        DebugDrawText::log("ERROR reading Sonic's yaw");
    }*/

    // Get Sonic's position (in string)
    size_t xPosStart = str.find("<x>");
    size_t xPposEnd = str.find("</x>");
    size_t yPosStart = str.find("<y>");
    size_t yPposEnd = str.find("</y>");
    size_t zPosStart = str.find("<z>");
    size_t zPposEnd = str.find("</z>");
    if (xPosStart != std::string::npos && xPposEnd != std::string::npos
     && yPosStart != std::string::npos && yPposEnd != std::string::npos
     && zPosStart != std::string::npos && zPposEnd != std::string::npos)
    {
        Chip::m_sonicStartPos.x() = std::stof(str.substr(xPosStart + 3, xPposEnd - xPosStart - 3));
        Chip::m_sonicStartPos.y() = std::stof(str.substr(yPosStart + 3, yPposEnd - yPosStart - 3));
        Chip::m_sonicStartPos.z() = std::stof(str.substr(zPosStart + 3, zPposEnd - zPosStart - 3));
        DebugDrawText::log(format("Sonic's pos = {%.3f, %.3f, %.3f}", Chip::m_sonicStartPos.x(), Chip::m_sonicStartPos.y(), Chip::m_sonicStartPos.z()));
    }
    else
    {
        Chip::m_startMode = StartMode::INVALID;
        DebugDrawText::log("ERROR reading Sonic's position");
    }

    return originalParseStageStgXml(a1, a2, pData, size);
}

HOOK(void, __fastcall, ObjPhyMsgNotifyObjectEvent, 0xEA4F50, void* This, void* Edx, uint32_t a2)
{
    uint32_t* pEvent = (uint32_t*)(a2 + 16);
    uint32_t* pObject = (uint32_t*)This;
    switch (*pEvent)
    {
    case 69:
    {
        // Use this to collect object pointer to result Chip object physics
        printf("[Chip Result] Added object physics ptr: 0x%08x (Type = %s)\n", (uint32_t)pObject, (char*)pObject[76]);
        Chip::m_chipResult.m_pResultList[std::string((char*)pObject[76])] = pObject;
        break;
    }
    case 70:
    {
        // Use this to play start animation
        if (*pModernSonicContext)
        {
            // Play starting animation as normal
            printf("[Chip Start] Playing start animation\n");
            *pEvent = 13;

            // Teleport to Sonic
            Eigen::Vector3f targetPosition;
            Eigen::Quaternionf targetRotation;
            if (Chip::getSonicTransform(targetPosition, targetRotation))
            {
                Chip::teleportTo(pObject, targetPosition, targetRotation);
                Chip::setChipEyeState(Chip::ChipEyeState::Stop);
            }
        }
        else
        {
            // Destroy it
            printf("[Chip Start] Not Modern Sonic, deleting starting Chip\n");
            *pEvent = 12;
        }
        break;
    }
    case 71:
    {
        if (*pModernSonicContext)
        {
            // At starting animation, delay Chip following
            printf("[Chip Follow] At starting animation, delaying Chip following\n");
            Chip::m_chipFollow.m_delayStart = true;
        }
        else
        {
            printf("[Chip Follow] Not Modern Sonic, starting Chip following immediately\n");
        }
        break;
    }
    case 72:
    {
        // Starting animation finished, resume Chip following
        printf("[Chip Follow] Starting animation finished, resuming Chip following\n");
        Chip::m_chipFollow.m_delayStart = false;

        // Play animation to reset frame time to 0
        printf("[Chip Follow] Reset follow animation\n");
        *pEvent = 13;
        break;
    }
    case 80:
    {
        if (Chip::m_chipFollow.m_pObject != pObject)
        {
            // Use this to get the following chip object
            printf("[Chip Follow] Updated following Chip ptr: 0x%08x (Type = %s)\n", (uint32_t)pObject, (char*)pObject[76]);
            Chip::m_chipFollow.reset();
            Chip::m_chipFollow.m_pObject = pObject;

            // Teleport to somewhere offscreen close to Sonic
            Chip::getSonicTransform(Chip::m_chipFollow.m_position, Chip::m_chipFollow.m_rotation);
            Chip::m_chipFollow.m_position += Chip::m_chipFollow.m_rotation * Eigen::Vector3f(0.7f, 15.0f, -0.7f);
            Chip::teleportTo(Chip::m_chipFollow.m_pObject, Chip::m_chipFollow.m_position, Chip::m_chipFollow.m_rotation);
            Chip::setChipEyeState(Chip::ChipEyeState::Stop);

            // Play animation to reset frame time to 0
            printf("[Chip Follow] Reset follow animation\n");
            *pEvent = 13;
        }
        break;
    }
    default: break;
    }

    originalObjPhyMsgNotifyObjectEvent(This, Edx, a2);
}

FUNCTION_PTR(void*, __thiscall, processMsgNotifyObjectEvent, 0xEA4F50, void* This, void* message);
HOOK(void, __fastcall, MsgChangeResultState, 0xE692C0, void* This, void* Edx, uint32_t a2)
{
    uint32_t const state = *(uint32_t*)(a2 + 16);
    if (state == 3)
    {
        void* pSonicContext = nullptr;
        if (!pSonicContext) pSonicContext = *pModernSonicContext;
        if (!pSonicContext) pSonicContext = *pClassicSonicContext;
        if (pSonicContext)
        {
            // Play only when it's not Super Sonic
            uint32_t superSonicAddress = (uint32_t)(pSonicContext) + 0x1A0;
            if (!*(void**)superSonicAddress)
            {
                uint32_t rank = *(uint32_t*)(a2 + 20);

                // Force rank animation code
                switch (*(uint8_t*)0xE693A7)
                {
                    case 0x90: rank = 4; break;
                    case 0x0C: rank = 3; break;
                    case 0x18: rank = 2; break;
                    case 0x24: rank = 1; break;
                    case 0x2B: rank = 0; break;
                    default: break; // 0x07
                }

                // E-Rank Generations support
                if (rank == 4 && *(uint8_t*)0x15EFE9D == 0x45)
                {
                    rank = 5;
                }

                std::string rankAnimStr = Chip::getChipRankAnimationName(rank);
                printf("[Chip Result] Rank animation = %s\n", rankAnimStr.c_str());
                auto iter = Chip::m_chipResult.m_pResultList.find(rankAnimStr);
                if (iter != Chip::m_chipResult.m_pResultList.end())
                {
                    // Cache result object
                    Chip::m_chipResult.m_pResult = iter->second;
                    Chip::m_chipResult.m_rank = rank;
                }
                else
                {
                    printf("[Chip Result] Object not found, base layer maybe disabled by the stage\n");
                }
            }
        }
    }

    originalMsgChangeResultState(This, Edx, a2);
}

HOOK(void, __fastcall, CSonicUpdate, 0xE6BF20, void* This, void* Edx, float* dt)
{
    originalCSonicUpdate(This, Edx, dt);

    Chip::m_chipFollow.advanceFollowSonic(*dt);
    Chip::m_chipResult.advanceResult();
}

HOOK(void, __fastcall, MsgPlayerGoal, 0xE692C0, void* This, void* Edx, uint32_t a2)
{
    if (Chip::m_chipFollow.m_pObject)
    {
        printf("[Chip Follow] Player hit goalring, leaving\n");
        Chip::m_chipFollow.m_goalLeave = true;
        Chip::m_chipFollow.m_goalLeaveYAdd = 0.0f;
    }

    originalMsgPlayerGoal(This, Edx, a2);
}

HOOK(void, __fastcall, CAudoDrawAdvance, 0x57C380, Chip::ChipEye* This, void* Edx, float dt)
{
    if (!Chip::m_chipEyeL || !Chip::m_chipEyeR)
    {
        std::string name((char*)(This->pData[2]));
        if (name.find("whip_gm_eyeL") != std::string::npos)
        {
            This->frameTime = 0;
            Chip::m_chipEyeL = This;
            printf("[Chip Eye] Updated Chip EyeL ptr: 0x%08x (Type = %s)\n", (uint32_t)This, name.c_str());
        }
        else if (name.find("whip_gm_eyeR") != std::string::npos)
        {
            This->frameTime = 0;
            Chip::m_chipEyeR = This;
            printf("[Chip Eye] Updated Chip EyeR ptr: 0x%08x (Type = %s)\n", (uint32_t)This, name.c_str());
        }
    }

    if (Chip::m_chipEyeL == This || Chip::m_chipEyeR == This)
    {
        // Stop advancing at certain area of uv-anim
        for (float endTime : Chip::m_chipEyeEndTimes)
        {
            if (This->frameTime >= endTime - 1 && This->frameTime < endTime)
            {
                dt = 0;
                break;
            }
        }
    }

    originalCAudoDrawAdvance(This, Edx, dt);
}

HOOK(int, __fastcall, CGameObject3DDestruction, 0xD5D790, void* This, void* Edx)
{
    if (Chip::m_chipFollow.m_pObject == This)
    {
        printf("[Chip Follow] DESTRUCTED\n");
        Chip::m_chipFollow.reset();
    }

    if (!Chip::m_chipResult.m_pResultList.empty())
    {
        bool destructResultObjects = false;
        for (auto iter : Chip::m_chipResult.m_pResultList)
        {
            if (iter.second == This)
            {
                destructResultObjects = true;
                break;
            }
        }

        if (destructResultObjects)
        {
            printf("[Chip Result] DESTRUCTED\n");
            Chip::m_chipResult.reset();
        }
    }

    return originalCGameObject3DDestruction(This, Edx);
}

void Chip::applyPatches()
{
    // Load Chip object physics
    INSTALL_HOOK(LoadObjectProduction);

    // Get Sonic's starting position and mode
    INSTALL_HOOK(ParseStageStgXml);

    // Use event to get the Chip rank obj phy ptr
    INSTALL_HOOK(ObjPhyMsgNotifyObjectEvent);

    // Teleport and plays Chip's rank animation
    INSTALL_HOOK(MsgChangeResultState);

    // Use this to make Chip follow Sonic
    INSTALL_HOOK(CSonicUpdate);

    // Detect when Sonic hits goal ring
    INSTALL_HOOK(MsgPlayerGoal);

    // Manipulate autodraw for Chip's eyes
    INSTALL_HOOK(CAudoDrawAdvance);

    // Check when Chip's object physics are destructed
    INSTALL_HOOK(CGameObject3DDestruction);

    // Use Unleashed goal camera (default) param to avoid blocking Chip with HUD
    // CameraSp -> CameraSu (this doesn't read CameraSu, just fall back to default values)
    WRITE_MEMORY(0x15AF4C3, uint8_t, 0x75);
    WRITE_MEMORY(0x15D1EBB, uint8_t, 0x75);
    WRITE_MEMORY(0x15D293B, uint8_t, 0x75);
    WRITE_MEMORY(0x1A48C7C, float, -0.5753f); // -0.05 from default OffsetRight

    // Disable transition between Result_Link and Result_LinkL
    // WRITE_NOP(0xDA59D5, 0x1F3); // Modern Super
    // WRITE_NOP(0xDAB679, 0x20B); // Classic Super
    WRITE_NOP(0xDDD7A9, 0x244); // Classic
    WRITE_NOP(0xE19D98, 0x1E5); // Modern
}

uint32_t m_currentSetObjectID = 5000000;
uint32_t getNextAvailableSetObjectID(std::set<uint32_t> const& usedSetObjectIDs)
{
    m_currentSetObjectID++;
    while (usedSetObjectIDs.find(m_currentSetObjectID) != usedSetObjectIDs.end())
    {
        m_currentSetObjectID++;
    }
    DebugDrawText::log(format("Next available SetObjectID = %u", m_currentSetObjectID));
    return m_currentSetObjectID;
}

bool Chip::generateChipSetdata(std::set<uint32_t> const& usedSetObjectIDs, std::string& chipSetdataStr)
{
    DebugDrawText::log("Generating Chip setdata...");
    chipSetdataStr.clear();

    bool const atHubWorld = getAtHubWorld();

    // -----------------------------------------------------------------------
    // Add chip following object
    uint32_t followChipID = 0;
    if ((atHubWorld && Configuration::m_followHub) || (!atHubWorld && Configuration::m_followStage))
    {
        followChipID = getNextAvailableSetObjectID(usedSetObjectIDs);
        chipSetdataStr += "<ObjectPhysics>\n";
        chipSetdataStr += "    <AddRange>0</AddRange>\n";
        chipSetdataStr += "    <CullingRange>100000</CullingRange>\n";
        chipSetdataStr += "    <DebrisTarget>\n";
        chipSetdataStr += "        <x>0</x>\n";
        chipSetdataStr += "        <y>0</y>\n";
        chipSetdataStr += "        <z>0</z>\n";
        chipSetdataStr += "    </DebrisTarget>\n";
        chipSetdataStr += "    <GroundOffset>0</GroundOffset>\n";
        chipSetdataStr += "    <IsCastShadow>true</IsCastShadow>\n";
        chipSetdataStr += "    <IsDynamic>false</IsDynamic>\n";
        chipSetdataStr += "    <IsReset>false</IsReset>\n";
        chipSetdataStr += "    <Range>100000</Range>\n";
        chipSetdataStr += "    <SetObjectID>" + std::to_string(followChipID) + "</SetObjectID>\n";
        chipSetdataStr += "    <Type>chip_idle</Type>\n";
        chipSetdataStr += "    <WrappedObjectID>\n";
        chipSetdataStr += "        <SetObjectID>0</SetObjectID>\n";
        chipSetdataStr += "    </WrappedObjectID>\n";
        chipSetdataStr += "    <Position>\n";
        chipSetdataStr += "        <x>0</x>\n";
        chipSetdataStr += "        <y>10000</y>\n";
        chipSetdataStr += "        <z>0</z>\n";
        chipSetdataStr += "    </Position>\n";
        chipSetdataStr += "    <Rotation>\n";
        chipSetdataStr += "        <x>0</x>";
        chipSetdataStr += "        <y>0</y>";
        chipSetdataStr += "        <z>0</z>";
        chipSetdataStr += "        <w>1</w>";
        chipSetdataStr += "    </Rotation>\n";
        chipSetdataStr += "</ObjectPhysics>\n";

        uint32_t const followEventColID = getNextAvailableSetObjectID(usedSetObjectIDs);
        chipSetdataStr += "<EventCollision>\n";
        chipSetdataStr += "    <Collision_Height>100000</Collision_Height>\n";
        chipSetdataStr += "    <Collision_Length>100000</Collision_Length>\n";
        chipSetdataStr += "    <Collision_Width>100000</Collision_Width>\n";
        chipSetdataStr += "    <DefaultStatus>0</DefaultStatus>\n";
        chipSetdataStr += "    <Durability>1</Durability>\n";
        chipSetdataStr += "    <Event0>80</Event0>\n";;
        chipSetdataStr += "    <Event1>0</Event1>\n";
        chipSetdataStr += "    <Event2>0</Event2>\n";
        chipSetdataStr += "    <Event3>0</Event3>\n";
        chipSetdataStr += "    <GroundOffset>0</GroundOffset>\n";
        chipSetdataStr += "    <IsCastShadow>true</IsCastShadow>\n";
        chipSetdataStr += "    <Range>100000</Range>\n";
        chipSetdataStr += "    <SetObjectID>" + std::to_string(followEventColID) + "</SetObjectID>\n";
        chipSetdataStr += "    <Shape_Type>0</Shape_Type>\n";
        chipSetdataStr += "    <TargetList0>\n";
        chipSetdataStr += "        <SetObjectID>" + std::to_string(followChipID) + "</SetObjectID>\n";
        chipSetdataStr += "    </TargetList0>\n";
        chipSetdataStr += "    <TargetList1 />\n";
        chipSetdataStr += "    <TargetList2 />\n";
        chipSetdataStr += "    <TargetList3 />\n";
        chipSetdataStr += "    <Timer0>0</Timer0>\n";
        chipSetdataStr += "    <Timer1>0</Timer1>\n";
        chipSetdataStr += "    <Timer2>0</Timer2>\n";
        chipSetdataStr += "    <Timer3>0</Timer3>\n";
        chipSetdataStr += "    <Trigger>\n";
        chipSetdataStr += "        <SetObjectID>0</SetObjectID>\n";
        chipSetdataStr += "    </Trigger>\n";
        chipSetdataStr += "    <TriggerType>0</TriggerType>\n";
        chipSetdataStr += "    <Position>";
        chipSetdataStr += "        <x>0</x>";
        chipSetdataStr += "        <y>0</y>";
        chipSetdataStr += "        <z>0</z>";
        chipSetdataStr += "    </Position>\n";
        chipSetdataStr += "    <Rotation>\n";
        chipSetdataStr += "        <x>0</x>";
        chipSetdataStr += "        <y>0</y>";
        chipSetdataStr += "        <z>0</z>";
        chipSetdataStr += "        <w>1</w>";
        chipSetdataStr += "    </Rotation>\n";
        chipSetdataStr += "</EventCollision>\n";
    }

    // Hub world no need to add start Chip or result Chip
    if (atHubWorld) return true;

    // -----------------------------------------------------------------------
    // Add starting Chip object
    if (Chip::m_startMode != StartMode::INVALID)
    {
        uint32_t const startChipID = getNextAvailableSetObjectID(usedSetObjectIDs);
        chipSetdataStr += "<ObjectPhysics>\n";
        chipSetdataStr += "    <AddRange>0</AddRange>\n";
        chipSetdataStr += "    <CullingRange>100000</CullingRange>\n";
        chipSetdataStr += "    <DebrisTarget>\n";
        chipSetdataStr += "        <x>0</x>\n";
        chipSetdataStr += "        <y>0</y>\n";
        chipSetdataStr += "        <z>0</z>\n";
        chipSetdataStr += "    </DebrisTarget>\n";
        chipSetdataStr += "    <GroundOffset>0</GroundOffset>\n";
        chipSetdataStr += "    <IsCastShadow>true</IsCastShadow>\n";
        chipSetdataStr += "    <IsDynamic>false</IsDynamic>\n";
        chipSetdataStr += "    <IsReset>false</IsReset>\n";
        chipSetdataStr += "    <Range>100000</Range>\n";
        chipSetdataStr += "    <SetObjectID>";
        chipSetdataStr += std::to_string(startChipID);
        chipSetdataStr += "</SetObjectID>\n";
        chipSetdataStr += "    <Type>" + std::string(Configuration::m_modelType == 1 ? "chip_start_evil" : (m_startMode == StartMode::Stand ? "chip_start_sonicI" : "chip_start_sonicD")) + "</Type>\n";
        chipSetdataStr += "    <WrappedObjectID>\n";
        chipSetdataStr += "        <SetObjectID>0</SetObjectID>\n";
        chipSetdataStr += "    </WrappedObjectID>\n";
        chipSetdataStr += "    <Position>\n";
        chipSetdataStr += "        <x>0</x>";
        chipSetdataStr += "        <y>10000</y>";
        chipSetdataStr += "        <z>0</z>";
        chipSetdataStr += "    </Position>\n";
        chipSetdataStr += "    <Rotation>\n";
        chipSetdataStr += "        <x>0</x>";
        chipSetdataStr += "        <y>0</y>";
        chipSetdataStr += "        <z>0</z>";
        chipSetdataStr += "        <w>1</w>";
        chipSetdataStr += "    </Rotation>\n";
        chipSetdataStr += "</ObjectPhysics>\n";

        uint32_t const startEventColID = getNextAvailableSetObjectID(usedSetObjectIDs);
        chipSetdataStr += "<EventCollision>\n";
        chipSetdataStr += "    <Collision_Height>3</Collision_Height>\n";
        chipSetdataStr += "    <Collision_Length>3</Collision_Length>\n";
        chipSetdataStr += "    <Collision_Width>3</Collision_Width>\n";
        chipSetdataStr += "    <DefaultStatus>0</DefaultStatus>\n";
        chipSetdataStr += "    <Durability>0</Durability>\n";
        chipSetdataStr += "    <Event0>70</Event0>\n";
        chipSetdataStr += "    <Event1>12</Event1>\n";
        chipSetdataStr += "    <Event2>71</Event2>\n";
        chipSetdataStr += "    <Event3>72</Event3>\n";
        chipSetdataStr += "    <GroundOffset>0</GroundOffset>\n";
        chipSetdataStr += "    <IsCastShadow>true</IsCastShadow>\n";
        chipSetdataStr += "    <Range>10030</Range>\n";
        chipSetdataStr += "    <SetObjectID>" + std::to_string(startEventColID) + "</SetObjectID>\n";
        chipSetdataStr += "    <Shape_Type>0</Shape_Type>\n";
        chipSetdataStr += "    <TargetList0>\n";
        chipSetdataStr += "        <SetObjectID>" + std::to_string(startChipID) + "</SetObjectID>\n";
        chipSetdataStr += "    </TargetList0>\n";
        chipSetdataStr += "    <TargetList1>\n";
        chipSetdataStr += "        <SetObjectID>" + std::to_string(startChipID) + "</SetObjectID>\n";
        chipSetdataStr += "    </TargetList1>\n";
        if (followChipID == 0)
        {
            chipSetdataStr += "    <TargetList2 />\n";
            chipSetdataStr += "    <TargetList3 />\n";
        }
        else
        {
            chipSetdataStr += "    <TargetList2>\n";
            chipSetdataStr += "        <SetObjectID>" + std::to_string(followChipID) + "</SetObjectID>\n";
            chipSetdataStr += "    </TargetList2>\n";
            chipSetdataStr += "    <TargetList3>\n";
            chipSetdataStr += "        <SetObjectID>" + std::to_string(followChipID) + "</SetObjectID>\n";
            chipSetdataStr += "    </TargetList3>\n";
        }
        chipSetdataStr += "    <Timer0>0</Timer0>\n";
        chipSetdataStr += "    <Timer1>" + std::string(Configuration::m_modelType == 1 ? "4" : (m_startMode == StartMode::Stand ? "3.8" : "3.4")) + "</Timer1>\n";
        chipSetdataStr += "    <Timer2>0</Timer2>\n";
        if (followChipID == 0)
        {
            chipSetdataStr += "    <Timer3>0</Timer3>\n";
        }
        else
        {
            chipSetdataStr += "    <Timer3>" + std::string(Configuration::m_modelType == 1 ? "4" : (m_startMode == StartMode::Stand ? "3.8" : "3.4")) + "</Timer3>\n";
        }
        chipSetdataStr += "    <Trigger>\n";
        chipSetdataStr += "        <SetObjectID>0</SetObjectID>\n";
        chipSetdataStr += "    </Trigger>\n";
        chipSetdataStr += "    <TriggerType>0</TriggerType>\n";
        chipSetdataStr += "    <Position>";
        chipSetdataStr += Chip::to_string(Chip::m_sonicStartPos);
        chipSetdataStr += "    </Position>\n";
        chipSetdataStr += "    <Rotation>\n";
        chipSetdataStr += "        <x>0</x>";
        chipSetdataStr += "        <y>0</y>";
        chipSetdataStr += "        <z>0</z>";
        chipSetdataStr += "        <w>1</w>";
        chipSetdataStr += "    </Rotation>\n";
        chipSetdataStr += "</EventCollision>\n";
    }

    // -----------------------------------------------------------------------
    // Add all ranking Chip object physics
    uint32_t const chipRankSID = getNextAvailableSetObjectID(usedSetObjectIDs);
    addChipRankObjectPhysics(chipSetdataStr, getChipRankAnimationName(4), chipRankSID);
    uint32_t const chipRankAID = getNextAvailableSetObjectID(usedSetObjectIDs);
    addChipRankObjectPhysics(chipSetdataStr, getChipRankAnimationName(3), chipRankAID);
    uint32_t const chipRankBID = getNextAvailableSetObjectID(usedSetObjectIDs);
    addChipRankObjectPhysics(chipSetdataStr, getChipRankAnimationName(2), chipRankBID);
    uint32_t const chipRankCID = getNextAvailableSetObjectID(usedSetObjectIDs);
    addChipRankObjectPhysics(chipSetdataStr, getChipRankAnimationName(1), chipRankCID);
    uint32_t const chipRankDID = getNextAvailableSetObjectID(usedSetObjectIDs);
    addChipRankObjectPhysics(chipSetdataStr, getChipRankAnimationName(0), chipRankDID);
    uint32_t const chipRankEID = getNextAvailableSetObjectID(usedSetObjectIDs);
    addChipRankObjectPhysics(chipSetdataStr, getChipRankAnimationName(5), chipRankEID);

    // This object have Durability 1 to update Chip object pointers whenever Sonic dies
    uint32_t const goalEventColID = getNextAvailableSetObjectID(usedSetObjectIDs);
    chipSetdataStr += "<EventCollision>\n";
    chipSetdataStr += "    <Collision_Height>100000</Collision_Height>\n";
    chipSetdataStr += "    <Collision_Length>100000</Collision_Length>\n";
    chipSetdataStr += "    <Collision_Width>100000</Collision_Width>\n";
    chipSetdataStr += "    <DefaultStatus>0</DefaultStatus>\n";
    chipSetdataStr += "    <Durability>1</Durability>\n";
    chipSetdataStr += "    <Event0>69</Event0>\n";;
    chipSetdataStr += "    <Event1>0</Event1>\n";
    chipSetdataStr += "    <Event2>0</Event2>\n";
    chipSetdataStr += "    <Event3>0</Event3>\n";
    chipSetdataStr += "    <GroundOffset>0</GroundOffset>\n";
    chipSetdataStr += "    <IsCastShadow>true</IsCastShadow>\n";
    chipSetdataStr += "    <Range>100000</Range>\n";
    chipSetdataStr += "    <SetObjectID>" + std::to_string(goalEventColID) + "</SetObjectID>\n";
    chipSetdataStr += "    <Shape_Type>0</Shape_Type>\n";
    chipSetdataStr += "    <TargetList0>\n";
    chipSetdataStr += "        <SetObjectID>" + std::to_string(chipRankSID) + "</SetObjectID>\n";
    chipSetdataStr += "        <SetObjectID>" + std::to_string(chipRankAID) + "</SetObjectID>\n";
    chipSetdataStr += "        <SetObjectID>" + std::to_string(chipRankBID) + "</SetObjectID>\n";
    chipSetdataStr += "        <SetObjectID>" + std::to_string(chipRankCID) + "</SetObjectID>\n";
    chipSetdataStr += "        <SetObjectID>" + std::to_string(chipRankDID) + "</SetObjectID>\n";
    chipSetdataStr += "        <SetObjectID>" + std::to_string(chipRankEID) + "</SetObjectID>\n";
    chipSetdataStr += "    </TargetList0>\n";
    chipSetdataStr += "    <TargetList1 />\n";
    chipSetdataStr += "    <TargetList2 />\n";
    chipSetdataStr += "    <TargetList3 />\n";
    chipSetdataStr += "    <Timer0>0</Timer0>\n";
    chipSetdataStr += "    <Timer1>0</Timer1>\n";
    chipSetdataStr += "    <Timer2>0</Timer2>\n";
    chipSetdataStr += "    <Timer3>0</Timer3>\n";
    chipSetdataStr += "    <Trigger>\n";
    chipSetdataStr += "        <SetObjectID>0</SetObjectID>\n";
    chipSetdataStr += "    </Trigger>\n";
    chipSetdataStr += "    <TriggerType>0</TriggerType>\n";
    chipSetdataStr += "    <Position>";
    chipSetdataStr += "        <x>0</x>";
    chipSetdataStr += "        <y>0</y>";
    chipSetdataStr += "        <z>0</z>";
    chipSetdataStr += "    </Position>\n";
    chipSetdataStr += "    <Rotation>\n";
    chipSetdataStr += "        <x>0</x>";
    chipSetdataStr += "        <y>0</y>";
    chipSetdataStr += "        <z>0</z>";
    chipSetdataStr += "        <w>1</w>";
    chipSetdataStr += "    </Rotation>\n";
    chipSetdataStr += "</EventCollision>\n";

    return true;
}

void Chip::addChipRankObjectPhysics(std::string& chipSetdataStr, std::string const& type, uint32_t const setObjectID)
{
    chipSetdataStr += "<ObjectPhysics>\n";
    chipSetdataStr += "    <AddRange>0</AddRange>\n";
    chipSetdataStr += "    <CullingRange>100000</CullingRange>\n";
    chipSetdataStr += "    <DebrisTarget>\n";
    chipSetdataStr += "        <x>0</x>\n";
    chipSetdataStr += "        <y>0</y>\n";
    chipSetdataStr += "        <z>0</z>\n";
    chipSetdataStr += "    </DebrisTarget>\n";
    chipSetdataStr += "    <GroundOffset>0</GroundOffset>\n";
    chipSetdataStr += "    <IsCastShadow>true</IsCastShadow>\n";
    chipSetdataStr += "    <IsDynamic>false</IsDynamic>\n";
    chipSetdataStr += "    <IsReset>false</IsReset>\n";
    chipSetdataStr += "    <Range>100000</Range>\n";
    chipSetdataStr += "    <SetObjectID>" + std::to_string(setObjectID) + "</SetObjectID>\n";
    chipSetdataStr += "    <Type>" + type + "</Type>\n";
    chipSetdataStr += "    <WrappedObjectID>\n";
    chipSetdataStr += "        <SetObjectID>0</SetObjectID>\n";
    chipSetdataStr += "    </WrappedObjectID>\n";
    chipSetdataStr += "    <Position>\n";
    chipSetdataStr += "        <x>0</x>\n";
    chipSetdataStr += "        <y>10000</y>\n";
    chipSetdataStr += "        <z>0</z>\n";
    chipSetdataStr += "    </Position>\n";
    chipSetdataStr += "    <Rotation>\n";
    chipSetdataStr += "        <x>0</x>\n";
    chipSetdataStr += "        <y>0</y>\n";
    chipSetdataStr += "        <z>0</z>\n";
    chipSetdataStr += "        <w>1</w>\n";
    chipSetdataStr += "    </Rotation>\n";
    chipSetdataStr += "</ObjectPhysics>\n";
}

std::string Chip::getChipRankAnimationName(uint32_t rank)
{
    std::string str;
    switch (rank)
    {
        case 5: str = "chip_rank_E"; break;
        case 4: str = "chip_rank_S"; break;
        case 3: str = "chip_rank_A"; break;
        case 2: str = "chip_rank_B"; break;
        case 1: str = "chip_rank_C"; break;
        default: str = "chip_rank_D"; break;
    }

    // Werehog
    if (Configuration::m_modelType == 1)
    {
        str += "_evil";
    }

    return str;
}

std::string Chip::getRotationStrFromYaw(float yaw)
{
    std::string str;

    str += "<x>0</x>\n";
    str += "<y>" + std::to_string(sin(yaw * PI / 360.0f)) + "</y>\n";
    str += "<z>0</z>\n";
    str += "<w>" + std::to_string(cos(yaw * PI / 360.0f)) + "</w>\n";

    return str;
}

std::string Chip::to_string(Eigen::Vector3f v)
{
    std::string str;
    str += "<x>" + std::to_string(v.x()) + "</x>\n";
    str += "<y>" + std::to_string(v.y()) + "</y>\n";
    str += "<z>" + std::to_string(v.z()) + "</z>\n";
    return str;
}

std::string Chip::to_string(Eigen::Quaternionf q)
{
    std::string str;
    str += "<x>" + std::to_string(q.x()) + "</x>\n";
    str += "<y>" + std::to_string(q.y()) + "</y>\n";
    str += "<z>" + std::to_string(q.z()) + "</z>\n";
    str += "<w>" + std::to_string(q.w()) + "</w>\n";
    return str;
}

void Chip::setChipEyeState(ChipEyeState state)
{
    if (!Chip::m_chipEyeL || !Chip::m_chipEyeR) return;

    float startTime = 0;
    if (state != ChipEyeState::Stop)
    {
        startTime = Chip::m_chipEyeEndTimes[state - 1];
    }

    printf("[Chip Eye] State = %u (%.0fs - %.0fs)\n", state, startTime, Chip::m_chipEyeEndTimes[state]);
    Chip::m_chipEyeL->frameTime = startTime;
    Chip::m_chipEyeR->frameTime = startTime;
}


float Chip::getSonicSpeedSquared()
{
    if (!*PLAYER_CONTEXT) return 0.0f;

    const uint32_t result = *(uint32_t*)((uint32_t) * (void**)((uint32_t)*PLAYER_CONTEXT + 0x110) + 172);
    if (!result) return 0.0f;

    float* velocity = (float*)(result + 656);
    return velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2];
}

bool Chip::getSonicTransform(Eigen::Vector3f& position, Eigen::Quaternionf& rotation)
{
    if (!*PLAYER_CONTEXT) return false;

    const uint32_t result = *(uint32_t*)((uint32_t) * (void**)((uint32_t)*PLAYER_CONTEXT + 0x110) + 172);
    if (!result) return false;

    float* pPos = (float*)(*(uint32_t*)(result + 16) + 112);
    position.x() = pPos[0];
    position.y() = pPos[1];
    position.z() = pPos[2];

    float* pRot = (float*)(*(uint32_t*)(result + 16) + 96);
    rotation.x() = pRot[0];
    rotation.y() = pRot[1];
    rotation.z() = pRot[2];
    rotation.w() = pRot[3];

    return true;
}

FUNCTION_PTR(void*, __thiscall, processObjectMsgSetPosition, 0xEA2130, void* This, void* message);
FUNCTION_PTR(void*, __thiscall, processObjectMsgSetRotation, 0xEA20D0, void* This, void* message);
void Chip::teleportTo(void* pObject, Eigen::Vector3f position, Eigen::Quaternionf rotation)
{
    if (!pObject) return;

    // Teleport to current Sonic's position & rotation
    struct MsgSetPosition
    {
        INSERT_PADDING(0x10);
        Eigen::Vector3f position;
        INSERT_PADDING(0x4);
    };

    alignas(16) MsgSetPosition msgSetPosition;
    msgSetPosition.position = position;
    //printf("Teleport position = {%.3f, %.3f, %.3f}", msgSetPosition.position.x(), msgSetPosition.position.y(), msgSetPosition.position.z());
    processObjectMsgSetPosition(pObject, &msgSetPosition);

    struct MsgSetRotation
    {
        INSERT_PADDING(0x10);
        Eigen::Quaternionf rotation;
    };

    alignas(16) MsgSetRotation msgSetRotation;
    msgSetRotation.rotation = rotation;
    //printf(", rotation = {%.3f, %.3f, %.3f, %.3f}\n", msgSetRotation.rotation.x(), msgSetRotation.rotation.y(), msgSetRotation.rotation.x(), msgSetRotation.rotation.w());
    processObjectMsgSetRotation(pObject, &msgSetRotation);
}

float getRandomFloat(float min, float max)
{
    return min + (float(rand()) / float(RAND_MAX) * (max - min));
}

void Chip::ChipResult::advanceResult()
{
    if (!m_pResult || m_rank == -1) return;

    // Delay teleport by 1 frame
    if (!m_teleported && m_state == ChipState::Result)
    {
        Eigen::Vector3f targetPosition;
        Eigen::Quaternionf targetRotation;
        if (Chip::getSonicTransform(targetPosition, targetRotation))
        {
            Chip::teleportTo(m_pResult, targetPosition, targetRotation);
            Chip::setChipEyeState(ChipEyeState::Stop);
        }
        m_teleported = true;
    }

    // Handle state change
    if (m_state == ChipState::None)
    {
        printf("[Chip Result] State change: None -> ResultLook\n");
        m_state = ChipState::Result;

        alignas(16) MsgNotifyObjectEvent msgNotifyObjectEvent;
        msgNotifyObjectEvent.event = 13;
        processMsgNotifyObjectEvent(m_pResult, &msgNotifyObjectEvent);
    }
    else
    {
        float const frameTime = Chip::updateGetFrameTime(m_pResult) * 30.0f;
        if (frameTime >= 99.0f) // 100 frames
        {
            printf("[Chip Result] State change: ResultLook -> Result\n");

            // Eye uv-anim
            ChipEyeState eyeState = ChipEyeState::Stop;
            switch (m_rank)
            {
                case 5: eyeState = ChipEyeState::Sonic_E; break;
                case 4: eyeState = ChipEyeState::Sonic_S; break;
                case 3: eyeState = ChipEyeState::Sonic_A; break;
                case 2: eyeState = ChipEyeState::Sonic_B; break;
                case 1: eyeState = ChipEyeState::Sonic_C; break;
                default: eyeState = ChipEyeState::Sonic_D; break;
            }
            if (Configuration::m_modelType == 1)
            {
                // Werehog results
                eyeState = (ChipEyeState)(eyeState + 6);
            }
            Chip::setChipEyeState(eyeState);

            // We are done
            reset();
            return;
        }
    }
}

void Chip::ChipFollow::advanceFollowSonic(float dt)
{
    if (!m_pObject || m_delayStart) return;

    Eigen::Vector3f targetPosition;
    Eigen::Quaternionf targetRotation;
    if (Chip::getSonicTransform(targetPosition, targetRotation))
    {
        float const sonicY = targetPosition.y();

        // Only affect Yaw rotation
        Eigen::Vector3f forward = targetRotation * Eigen::Vector3f(0, 0, 1);
        forward.y() = 0;
        if (forward.x() == 0 && forward.z() == 0)
        {
            forward.z() = 1;
        }
        targetRotation = Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f(0, 0, 1), forward);
        targetRotation.x() = 0;
        targetRotation.z() = 0;

        // Gradually increse the height after hitting goal ring
        float const maxGoalLeaveHeight = 15.0f;
        if (m_goalLeave)
        {
            m_goalLeaveYAdd = min(maxGoalLeaveHeight, m_goalLeaveYAdd + dt * 40.0f);
        }

        // Get relative position from Sonic, add randomness
        m_newPosTimer -= dt;
        if (m_newPosTimer <= 0.0f)
        {
            m_newPosTimer = 2.0f;
            m_randPosTarget = Eigen::Vector3f(getRandomFloat(-1.2f, 1.2f), getRandomFloat(1.0f, 1.8f), getRandomFloat(-1.1f, -0.5f));
        }
        m_randPosAdd = m_randPosAdd + (m_randPosTarget - m_randPosAdd) * min(dt * 0.5f, 1.0f);

        // Interpolate rotation first
        float const lerpRate = min(dt * 2.0f, 1.0f);
        m_rotation = m_rotation.slerp(lerpRate, targetRotation);
        
        // Interpolate position relative to rotation
        targetPosition += m_rotation * (m_randPosAdd + Eigen::Vector3f(0.0f, (m_goalLeave ? m_goalLeaveYAdd : 0.0f), 0.0f));
        Eigen::Vector3f distTravelling = (targetPosition - m_position) * lerpRate;
        m_position += distTravelling;

        // After goal if Chip is far away from Sonic, delete
        if (m_goalLeave && m_position.y() > sonicY + maxGoalLeaveHeight - 3.0f)
        {
            // Play animation
            printf("[Chip Follow] Destroying following Chip\n");
            alignas(16) MsgNotifyObjectEvent msgNotifyObjectEvent;
            msgNotifyObjectEvent.event = 12;
            processMsgNotifyObjectEvent(m_pObject, &msgNotifyObjectEvent);

            reset();
            return;
        }

        // Get current speed
        float const speed = distTravelling.norm() / dt;
        bool const isMoving = speed > 4.0f;
        //DebugDrawText::draw(format("Speed = %.3f", speed), { 0,0 }, 3);
        
        // Get horizontal speed, use for ChipState::MoveFast
        distTravelling.y() = 0;
        float const xySpeed = distTravelling.norm() / dt;
        bool const isFastMoving = xySpeed > 30.0f;

        // Check how long have we been idle
        if (m_state == ChipState::Follow && !isMoving)
        {
            m_idleTimer += dt;
        }
        else
        {
            m_idleTimer = 0;
        }

        // Handling state transition and animation looping
        m_frameTime = Chip::updateGetFrameTime(m_pObject) * 30.0f;
        switch (m_state)
        {
        case ChipState::Follow:
        {
            if (m_frameTime >= 30.0f)
            {
                if (m_idleTimer > 4.0f)
                {
                    float const r = getRandomFloat(0.0f, 5.0f);
                    m_state = r < 4.0f ? ChipState::IdleA : ChipState::IdleB;
                    Chip::updateGetFrameTime(m_pObject, (m_state == ChipState::IdleA ? 60.0f : 330.0f) / 30.0f);
                    printf("[Chip Follow] State change: Follow -> %s\n", m_state == ChipState::IdleA ? "IdleA" : "IdleB");
                }
                else if (isMoving)
                {
                    Chip::updateGetFrameTime(m_pObject, 570.0f / 30.0f);
                    m_state = ChipState::IdleMove;
                    printf("[Chip Follow] State change: Follow -> IdleMove\n");
                }
                else
                {
                    Chip::updateGetFrameTime(m_pObject, (m_frameTime - 30.0f) / 30.0f);
                }
            }
            break;
        }
        case ChipState::IdleA:
        {
            if (m_frameTime >= 300.0f)
            {
                // Jump to frame 30 so we can transition to IdleMove if needed
                Chip::updateGetFrameTime(m_pObject, (m_frameTime - 270.0f) / 30.0f);
                m_state = ChipState::Follow;
                printf("[Chip Follow] State change: IdleA -> Follow\n");
            }
            break;
        }
        case ChipState::IdleB:
        {
            if (m_frameTime >= 540.0f)
            {
                // Jump to frame 30 so we can transition to IdleMove if needed
                Chip::updateGetFrameTime(m_pObject, (m_frameTime - 510.0f) / 30.0f);
                m_state = ChipState::Follow;
                printf("[Chip Follow] State change: IdleB -> Follow\n");
            }
            break;
        }
        case ChipState::IdleMove:
        {
            if (m_frameTime >= 585.0f)
            {
                // Jump to frame 30 so we can transition to MoveIdle if needed
                Chip::updateGetFrameTime(m_pObject, (m_frameTime + 30.0f) / 30.0f);
                m_state = ChipState::Move;
                printf("[Chip Follow] State change: IdleMove -> Move\n");
            }
            break;
        }
        case ChipState::Move:
        {
            if (m_frameTime >= 615.0f)
            {
                if (!isMoving)
                {
                    Chip::updateGetFrameTime(m_pObject, 645.0f / 30.0f);
                    m_state = ChipState::MoveIdle;
                    printf("[Chip Follow] State change: Move -> MoveIdle\n");
                }
                else
                {
                    Chip::updateGetFrameTime(m_pObject, (m_frameTime - 30.0f) / 30.0f);
                }
            }
            break;
        }
        case ChipState::MoveIdle:
        {
            if (m_frameTime >= 660.0f)
            {
                // Jump to frame 30 so we can transition to IdleMove if needed
                Chip::updateGetFrameTime(m_pObject, (m_frameTime - 630.0f) / 30.0f);
                m_state = ChipState::Follow;
                printf("[Chip Follow] State change: MoveIdle -> Follow\n");
            }
            break;
        }
        default: break;
        }

        // Finally teleport
        Chip::teleportTo(m_pObject, m_position, m_rotation);
    }
}

uint32_t GetMultiLevelAddress(uint32_t initAddress, vector<uint32_t> offsets)
{
    uint32_t address = *(uint32_t*)initAddress;
    for (uint32_t i = 0; i < offsets.size(); i++)
    {
        if (!address) return 0;

        uint32_t const& offset = offsets[i];
        address += offset;

        if (i < offsets.size() - 1)
        {
            address = *(uint32_t*)address;
        }
    }
    return address;
}

float Chip::updateGetFrameTime(void* pObject, float setFrameTime)
{
    if (pObject)
    {
        uint32_t frameTimeAddress = GetMultiLevelAddress((uint32_t)pObject + 0x110, { 0x0, 0x110, 0x4C, 0x8, 0x8 });
        if (frameTimeAddress)
        {
            if (setFrameTime >= 0.0f)
            {
                *(float*)frameTimeAddress = setFrameTime;
                return setFrameTime;
            }
            else
            {
                return *(float*)frameTimeAddress;
            }
        }
    }

    return FLT_MAX;
}
