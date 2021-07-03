#pragma once

struct ArchiveDependency
{
    string m_archive;
    vector<string> m_dependencies;

    ArchiveDependency() {};
    ArchiveDependency(string _archive, vector<string> _dependencies)
        : m_archive(_archive)
        , m_dependencies(_dependencies)
    {}
};

class ArchiveTreePatcher
{
public:
    static vector<ArchiveDependency> m_archiveDependencies;
    static void applyPatches();
};
