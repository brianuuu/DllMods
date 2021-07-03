#include "SetdataPatcher.h"
#include "Chip.h"
#include "Configuration.h"

std::string m_fileName;
HOOK(bool, __stdcall, ParseSetdata, 0xEB5050, void* a1, char** pFileName, void* a3, void* a4, uint8_t a5, uint8_t a6)
{
    m_fileName = std::string(*(char**)pFileName);
    //DebugDrawText::log(format("Reading setdata: %s", m_fileName.c_str()));

    bool result = originalParseSetdata(a1, pFileName, a3, a4, a5, a6);
    m_fileName.clear();

    return result;
}

HOOK(uint32_t*, __fastcall, ReadXmlData, 0xCE5FC0, uint32_t size, char* pData, void* a3, uint32_t* a4)
{
    /*std::string layerFile;
    switch (Configuration::m_injectLayer)
    {
        default: layerFile = "setdata_base.set.xml"; break;
        case 1: layerFile = "setdata_design.set.xml"; break;
        case 2: layerFile = "setdata_effect.set.xml"; break;
        case 3: layerFile = "setdata_sound.set.xml"; break;
    }*/

    if (m_fileName != "setdata_base.set.xml")
    {
        return originalReadXmlData(size, pData, a3, a4);
    }

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
            uint32_t setObjectID = std::stoi(setObjectIDStr);
            usedSetObjectIDs.insert(setObjectID);
            //printf("%u\n", setObjectID);
        }

        setObjectIDStart = originalSetdataStr.find("<SetObjectID>", setObjectIDEnd);
    }

    std::string chipSetdataStr;
    if (!Chip::generateChipSetdata(usedSetObjectIDs, chipSetdataStr))
    {
        return originalReadXmlData(size, pData, a3, a4);
    }

    DebugDrawText::log(format("Injecting Chip setdata to %s...", m_fileName.c_str()));

    const size_t newSize = size + chipSetdataStr.size();
    const std::unique_ptr<char[]> pBuffer = std::make_unique<char[]>(newSize);
    memcpy(pBuffer.get(), pData, size);

    char* pInsertionPos = strstr(pBuffer.get(), "</SetObject>");

    memmove(pInsertionPos + chipSetdataStr.size(), pInsertionPos, size - (size_t)(pInsertionPos - pBuffer.get()));
    memcpy(pInsertionPos, chipSetdataStr.c_str(), chipSetdataStr.size());

    return originalReadXmlData(newSize, pBuffer.get(), a3, a4);
}

void SetdataPatcher::applyPatches()
{
    INSTALL_HOOK(ParseSetdata);
    INSTALL_HOOK(ReadXmlData);
}
