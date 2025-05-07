//-----------------------------------------------------
// Name: mst.h
// Author: brianuuu
// Date: 29/7/2019
//-----------------------------------------------------

#pragma once
#include <string>
#include <vector>
#include <map>

using namespace std;

class mst
{
public:
    struct TextEntry
    {
        string m_name;
        vector<wstring> m_subtitles;
        vector<string> m_tags;
    };

public:
    mst();
    ~mst();

    bool IsLoaded() const { return m_loaded; }

    // Load & Save
    bool Load(string const& _fileName, string& _errorMsg);
    bool Save(string const& _fileName, string& _errorMsg);

    // Export plain text
    void Export(string const& _fileName, bool _russian = false);

    // Helpers
    int Search(string const& _str, unsigned int _start = 0) const;
    int Search(wstring const& _str, unsigned int _start = 0) const;
    void GetAllEntries(vector<TextEntry>& _textEntries) const;
    TextEntry GetEntry(unsigned int _id) const;
    TextEntry GetEntry(string const& _str) const;

    // Modifiers
    int AddNewEntry();
    void RemoveEntry(unsigned int _id);
    void ModifyEntry(unsigned int _id, TextEntry const& _entry);

private:
    // Reading from bytes
    unsigned int ReadInt(FILE* _file);
    string ReadAscii(FILE* _file, unsigned int _length = 0);
    wstring ReadUTF16(FILE* _file);

    // Writing bytes
    void WriteInt(FILE* _file, unsigned int _writeInt);
    void WriteAscii(FILE* _file, string _writeString, bool _termination = true);
    void WriteUTF16(FILE* _file, wstring _writeString, bool _termination = true);

private:
    bool m_loaded;
    unsigned int m_fileSize;

    string m_tableName;
    vector<TextEntry> m_entries;

    map<wchar_t, wchar_t> m_unicodeToRussian;
    map<wchar_t, wchar_t> m_russianToUnicode;
};

