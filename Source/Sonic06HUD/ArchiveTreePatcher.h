#pragma once

struct ArchiveDependency
{
    std::string m_archive;
    std::vector<std::string> m_dependencies;

    ArchiveDependency() {};
    ArchiveDependency(std::string _archive, std::vector<std::string> _dependencies)
        : m_archive(_archive)
        , m_dependencies(_dependencies)
    {}
};

class ArchiveTreePatcher
{
public:
    static std::vector<ArchiveDependency> m_archiveDependencies;
    static void applyPatches();
};
