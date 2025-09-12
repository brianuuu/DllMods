#include "ArchiveTreePatcher.h"
#include "Configuration.h"

std::vector<ArchiveDependency> ArchiveTreePatcher::m_archiveDependencies = {};

HOOK(bool, __stdcall, ParseArchiveTree, 0xD4C8E0, void* A1, char* pData, const size_t size, void* pDatabase)
{
    std::string originalData(pData, size);
    std::string str;
    {
        std::stringstream stream;

        for (ArchiveDependency const& node : ArchiveTreePatcher::m_archiveDependencies)
        {
            if (originalData.find(node.m_archive) != std::string::npos)
            {
                printf("[ArchiveTreePatcher] Already injected \"%s\" archive!\n", node.m_archive.c_str());
                continue;
            }

            printf("[ArchiveTreePatcher] Injecting \"%s\" archive\n", node.m_archive.c_str());
            stream << "  <Node>\n";
            stream << "    <Name>" << node.m_archive << "</Name>\n";
            stream << "    <Archive>" << node.m_archive << "</Archive>\n";
            stream << "    <Order>" << 0 << "</Order>\n";
            stream << "    <DefAppend>" << node.m_archive << "</DefAppend>\n";

            for (std::string const& dependency : node.m_dependencies)
            {
                stream << "    <Node>\n";
                stream << "      <Name>" << dependency << "</Name>\n";
                stream << "      <Archive>" << dependency << "</Archive>\n";
                stream << "      <Order>" << 0 << "</Order>\n";
                stream << "    </Node>\n";
            }

            stream << "  </Node>\n";
        }

        str = stream.str();
    }

    const size_t newSize = size + str.size();
    const std::unique_ptr<char[]> pBuffer = std::make_unique<char[]>(newSize);
    memcpy(pBuffer.get(), pData, size);

    char* pInsertionPos = strstr(pBuffer.get(), "<Include>");

    memmove(pInsertionPos + str.size(), pInsertionPos, size - (size_t)(pInsertionPos - pBuffer.get()));
    memcpy(pInsertionPos, str.c_str(), str.size());

    bool result;
    {
        result = originalParseArchiveTree(A1, pBuffer.get(), newSize, pDatabase);
    }

    return result;
}

void ArchiveTreePatcher::applyPatches()
{
    // Usage: m_archiveDependencies.push_back(ArchiveDependency( ARCHIVE, { DEPEND_ON_1, DEPEND_ON_2, ... }));

    if (Configuration::m_model == Configuration::ModelType::Shadow && Configuration::Shadow::m_shaodwDPad == Configuration::ShadowDPadType::Vehicles)
    {
        m_archiveDependencies.push_back(ArchiveDependency("VehicleAdd", {"SonicActionCommon"}));
    }

    if (!m_archiveDependencies.empty())
    {
        INSTALL_HOOK(ParseArchiveTree);
    }
}