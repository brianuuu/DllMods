#include "SetdataPatcher.h"

Eigen::Vector3f m_sonicStartPos(0, 0, 0);
HOOK(uint32_t, __stdcall, ParseStageStgXml, 0x11D27A0, void* a1, void* a2, char* pData, const size_t size)
{
    std::string str(pData, size);
    
    // Get Sonic's yaw
    float yaw = 0;
    size_t yawStart = str.find("<Yaw>");
    size_t yawEnd = str.find("</Yaw>");
    if (yawStart != std::string::npos && yawEnd != std::string::npos)
    {
        std::string yawStr = str.substr(yawStart + 5, yawEnd - yawStart - 5);
        yaw = std::stof(yawStr) * DEG_TO_RAD;
        printf("[Real Silver] Sonic's Yaw = %.3f", yaw);
    }

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
        m_sonicStartPos.x() = std::stof(str.substr(xPosStart + 3, xPposEnd - xPosStart - 3));
        m_sonicStartPos.y() = std::stof(str.substr(yPosStart + 3, yPposEnd - yPosStart - 3)) + 2.0f;
        m_sonicStartPos.z() = std::stof(str.substr(zPosStart + 3, zPposEnd - zPosStart - 3));

        Eigen::Vector3f forward = Eigen::AngleAxisf(yaw, Eigen::Vector3f::UnitY()) * Eigen::Vector3f::UnitZ();
        m_sonicStartPos += forward * 10.0f;

        printf("[Real Silver] Sonic's pos = {%.3f, %.3f, %.3f}", m_sonicStartPos.x(), m_sonicStartPos.y(), m_sonicStartPos.z());
    }

    return originalParseStageStgXml(a1, a2, pData, size);
}

std::string vector3f_to_string(Eigen::Vector3f v)
{
    std::string str;
    str += "<x>" + std::to_string(v.x()) + "</x>\n";
    str += "<y>" + std::to_string(v.y()) + "</y>\n";
    str += "<z>" + std::to_string(v.z()) + "</z>\n";
    return str;
}


std::string m_fileName;
HOOK(bool, __stdcall, ParseSetdata, 0xEB5050, void* a1, char** pFileName, void* a3, void* a4, uint8_t a5, uint8_t a6)
{
    m_fileName = std::string(*(char**)pFileName);
    //DebugDrawText::log(format("Reading setdata: %s", m_fileName.c_str()));

    bool result = originalParseSetdata(a1, pFileName, a3, a4, a5, a6);
    m_fileName.clear();

    return result;
}

void getNextAvailableSetObjectID(uint32_t& currentSetObjectID, std::set<uint32_t>& usedSetObjectIDs)
{
    while (usedSetObjectIDs.find(currentSetObjectID) != usedSetObjectIDs.end())
    {
        currentSetObjectID++;
    }
    usedSetObjectIDs.insert(currentSetObjectID);
}

HOOK(uint32_t*, __fastcall, ReadXmlData, 0xCE5FC0, uint32_t size, char* pData, void* a3, uint32_t* a4)
{
    // only inject in main stages
    if (Common::GetCurrentStageID() > SMT_pla200)
    {
        WRITE_MEMORY(0x1186231, uint8_t, 0x8B, 0x02);
        return originalReadXmlData(size, pData, a3, a4);
    }

    if (m_fileName != "setdata_base.set.xml")
    {
        return originalReadXmlData(size, pData, a3, a4);
    }

    // Force CMissionNPC_Appear to use Blaze
    WRITE_MEMORY(0x1186231, uint8_t, 0xB0, 0x0D);

    // Get all used set object IDs (This only avoids same ID in this layer, but better than nothing)
    std::set<uint32_t> usedSetObjectIDs;
    std::string originalSetdataStr(pData, size);
    size_t setObjectIDStart = originalSetdataStr.find("<SetObjectID>");
    while (setObjectIDStart != std::string::npos)
    {
        size_t setObjectIDEnd = originalSetdataStr.find("</SetObjectID>", setObjectIDStart);
        if (setObjectIDEnd != std::string::npos)
        {
            std::string setObjectIDStr = originalSetdataStr.substr(setObjectIDStart + 13, setObjectIDEnd - setObjectIDStart - 13);
            if (!setObjectIDStr.empty())
            {
                uint32_t setObjectID = std::stoi(setObjectIDStr);
                usedSetObjectIDs.insert(setObjectID);
                //printf("%u\n", setObjectID);
            }
        }

        setObjectIDStart = originalSetdataStr.find("<SetObjectID>", setObjectIDEnd);
    }

    uint32_t availableSetObjectID = 99998;
    std::string extraSetdataStr;
    extraSetdataStr += "<MissionNpcG_Blaze>\n";
    extraSetdataStr += "<AppearTime>0.3</AppearTime>\n";
    extraSetdataStr += "<EventRadius>3</EventRadius>\n";
    extraSetdataStr += "<GroundOffset>0</GroundOffset>\n";
    extraSetdataStr += "<HomingAspect>1</HomingAspect>\n";
    extraSetdataStr += "<HomingDistance>35</HomingDistance>\n";
    extraSetdataStr += "<HomingFovy>85</HomingFovy>\n";
    extraSetdataStr += "<HomingNear>1.2</HomingNear>\n";
    extraSetdataStr += "<HomingOffsetX>-1</HomingOffsetX>\n";
    extraSetdataStr += "<HomingOffsetY>1.2</HomingOffsetY>\n";
    extraSetdataStr += "<HomingRotationX>31</HomingRotationX>\n";
    extraSetdataStr += "<HomingSpeed>50.0998</HomingSpeed>\n";
    extraSetdataStr += "<IsCastShadow>true</IsCastShadow>\n";
    extraSetdataStr += "<Position>\n";
    extraSetdataStr += vector3f_to_string(m_sonicStartPos);
    extraSetdataStr += "</Position>\n";
    extraSetdataStr += "<Range>100000</Range>\n";
    extraSetdataStr += "<Rotation>\n";
    extraSetdataStr += "<w>1</w>\n";
    extraSetdataStr += "<x>0</x>\n";
    extraSetdataStr += "<y>0</y>\n";
    extraSetdataStr += "<z>0</z>\n";
    extraSetdataStr += "</Rotation>\n";
    getNextAvailableSetObjectID(availableSetObjectID, usedSetObjectIDs);
    extraSetdataStr += "<SetObjectID>" + std::to_string(availableSetObjectID) + "</SetObjectID>\n";
    extraSetdataStr += "</MissionNpcG_Blaze>\n";
    extraSetdataStr += "<MissionNpc_Appear>\n";
    extraSetdataStr += "<GroundOffset>0</GroundOffset>\n";
    extraSetdataStr += "<IsCastShadow>true</IsCastShadow>\n";
    extraSetdataStr += "<Position>\n";
    extraSetdataStr += vector3f_to_string(m_sonicStartPos);
    extraSetdataStr += "</Position>\n";
    extraSetdataStr += "<Range>100000</Range>\n";
    extraSetdataStr += "<Rotation>\n";
    extraSetdataStr += "<w>0.707107</w>\n";
    extraSetdataStr += "<x>0</x>\n";
    extraSetdataStr += "<y>-0.707107</y>\n";
    extraSetdataStr += "<z>0</z>\n";
    extraSetdataStr += "</Rotation>\n";
    getNextAvailableSetObjectID(availableSetObjectID, usedSetObjectIDs);
    extraSetdataStr += "<SetObjectID>" + std::to_string(availableSetObjectID) + "</SetObjectID>\n";
    extraSetdataStr += "<VanishObjList>\n";
    extraSetdataStr += "</VanishObjList>\n";
    extraSetdataStr += "</MissionNpc_Appear>\n";

    printf("[Real Silver] Injecting Blaze support setdata to %s...", m_fileName.c_str());

    const size_t newSize = size + extraSetdataStr.size();
    const std::unique_ptr<char[]> pBuffer = std::make_unique<char[]>(newSize);
    memcpy(pBuffer.get(), pData, size);

    char* pInsertionPos = strstr(pBuffer.get(), "</SetObject>");

    memmove(pInsertionPos + extraSetdataStr.size(), pInsertionPos, size - (size_t)(pInsertionPos - pBuffer.get()));
    memcpy(pInsertionPos, extraSetdataStr.c_str(), extraSetdataStr.size());

    return originalReadXmlData(newSize, pBuffer.get(), a3, a4);
}

void SetdataPatcher::applyPatches()
{
    INSTALL_HOOK(ParseStageStgXml);
    INSTALL_HOOK(ParseSetdata);
    INSTALL_HOOK(ReadXmlData);
}
