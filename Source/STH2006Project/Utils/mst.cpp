//-----------------------------------------------------
// Name: mst.cpp
// Author: brianuuu
// Date: 29/7/2019
//-----------------------------------------------------

#include "mst.h"

#include <assert.h>
#include <stdlib.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>
#include <codecvt>
#include <locale>

//-----------------------------------------------------
// Constructor
//-----------------------------------------------------
mst::mst()
{
    wstring unicode = L"¨ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ¸";
    wstring russian = L"ЁАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюяё";
    for(unsigned int i = 0; i < unicode.size(); i++)
    {
        m_unicodeToRussian[unicode[i]] = russian[i];
        m_russianToUnicode[russian[i]] = unicode[i];
    }

    m_loaded = false;
    m_fileSize = 0u;
}

//-----------------------------------------------------
// Destructor
//-----------------------------------------------------
mst::~mst()
{
}

//-----------------------------------------------------
// Load an fco file, return false if fail
//-----------------------------------------------------
bool mst::Load
(
    string const & _fileName,
    string & _errorMsg
)
{
    m_fileSize = 0;
    m_tableName.clear();
    m_entries.clear();
    m_loaded = false;

    FILE* mstFile;
    fopen_s(&mstFile, _fileName.c_str(), "rb");
    if (!mstFile)
    {
        _errorMsg = "Unable to open file!";
        return false;
    }

    m_fileSize = ReadInt(mstFile);
    fseek(mstFile, 0, SEEK_END);
    if (m_fileSize != (unsigned int)ftell(mstFile))
    {
        _errorMsg = "File size does not match the one stated in the file!";
        return false;
    }

    fseek(mstFile, 4, SEEK_SET);
    unsigned int offsetTableAddress = ReadInt(mstFile);
    unsigned int offsetTableSize = ReadInt(mstFile);

    // Padding
    fseek(mstFile, 0x0A, SEEK_CUR);

    // Check for 1BBINA <---- version 1, Big Endian, BINA format
    string verify1 = ReadAscii(mstFile, 6);
    if (verify1 != "1BBINA")
    {
        _errorMsg = "File is not 06 .mst file";
        return false;
    }

    // 00 00 00 00 Padding
    fseek(mstFile, 0x04, SEEK_CUR);

    // Check for WTXT
    string verify2 = ReadAscii(mstFile, 4);
    if (verify2 != "WTXT")
    {
        _errorMsg = "File is not 06 .mst file";
        return false;
    }

    unsigned int rootAddress = 0x20;

    // Read table name
    unsigned int nameAddress = ReadInt(mstFile);
    fseek(mstFile, rootAddress + nameAddress, SEEK_SET);
    m_tableName = ReadAscii(mstFile);

    // Read number of entries
    fseek(mstFile, rootAddress + 0x08, SEEK_SET);
    unsigned int entryCount = ReadInt(mstFile);

    // Read individual entries
    unsigned int currentAddress = ftell(mstFile);
    m_entries.reserve(entryCount);
    for (unsigned int i = 0; i < entryCount; ++i)
    {
        TextEntry newEntry;
        unsigned int nameAddress = ReadInt(mstFile);
        unsigned int subtitlesAddress = ReadInt(mstFile);
        unsigned int tagsAddress = ReadInt(mstFile);
        currentAddress = ftell(mstFile);

        // Read name
        fseek(mstFile, rootAddress + nameAddress, SEEK_SET);
        newEntry.m_name = ReadAscii(mstFile);

        // Read subtitle data, separate by '\f'
        fseek(mstFile, rootAddress + subtitlesAddress, SEEK_SET);
        wstring subtitles = ReadUTF16(mstFile);
        {
            // Separate individual tags
            size_t indexPrev = 0;
            size_t index = subtitles.find(L'\f', indexPrev);
            if (index == string::npos)
            {
                newEntry.m_subtitles.push_back(subtitles);
            }
            else
            {
                while (index != string::npos)
                {
                    wstring subString = subtitles.substr(indexPrev, index - indexPrev);
                    newEntry.m_subtitles.push_back(subString);
                    indexPrev = index + 1;
                    index = subtitles.find(L'\f', indexPrev);
                }

                wstring subString = subtitles.substr(indexPrev, subtitles.size() - indexPrev);
                newEntry.m_subtitles.push_back(subString);
            }
        }

        // Read all tags, address can be 0
        if (tagsAddress)
        {
            fseek(mstFile, rootAddress + tagsAddress, SEEK_SET);
            string tags = ReadAscii(mstFile);

            // Replace all "color," to "color),
            size_t c = 0;
            while (true)
            {
                 c = tags.find("color,", c);
                 if (c == std::string::npos) break;
                 tags.replace(c, 6, "color),"); // 6 = "color,"
                 c += 7; // 7 = "color),"
            }

            // Separate individual tags
            size_t indexPrev = 0;
            size_t index = tags.find("),", indexPrev);
            if (index == string::npos)
            {
                newEntry.m_tags.push_back(tags);
            }
            else
            {
                while (index != string::npos)
                {
                    index += 1; // "),"
                    string subString = tags.substr(indexPrev, index - indexPrev);

                    // Replace "color)" with "color"
                    if (subString == "color)")
                    {
                        subString = "color";
                    }

                    newEntry.m_tags.push_back(subString);
                    indexPrev = index + 1;
                    index = tags.find("),", indexPrev);
                }

                string subString = tags.substr(indexPrev, tags.size() - indexPrev);
                newEntry.m_tags.push_back(subString);
            }
        }

        m_entries.push_back(newEntry);

        // Go back to the offset reading address
        fseek(mstFile, currentAddress, SEEK_SET);
    }

    // Skip the offset table
    fseek(mstFile, rootAddress + offsetTableAddress, SEEK_SET);
    fseek(mstFile, offsetTableSize, SEEK_CUR);

    // We should reach the end of the file
    if (m_fileSize != (unsigned int)ftell(mstFile))
    {
        _errorMsg = "Unexpected file size!";
        return false;
    }

    fclose(mstFile);
    m_loaded = true;
    return true;
}

//-----------------------------------------------------
// Read an int from 4 bytes
//-----------------------------------------------------
unsigned int mst::ReadInt
(
    FILE * _file
)
{
    // Read int, require flipping bytes
    unsigned int flippedInt;
    fread(&flippedInt, sizeof(unsigned int), 1, _file);
    return _byteswap_ulong(flippedInt);
}

//-----------------------------------------------------
// Read char until 0x00, unless _length provided
//-----------------------------------------------------
string mst::ReadAscii
(
    FILE * _file,
    unsigned int _length
)
{
    string str;
    unsigned char chr;
    unsigned int index = 0;

    bool valid = false;
    while(ftell(_file) < (long)m_fileSize)
    {
        // Read fixed length
        if (_length > 0 && index == _length)
        {
            valid = true;
            break;
        }

        // Read one char
        fread(&chr, 1, 1, _file);

        // Append char to string or exit
        if (chr)
        {
            str += chr;
            index++;
        }
        else
        {
            valid = !str.empty();
            break;
        }
    }

    return valid ? str : string();
}

//-----------------------------------------------------
// Read UTF16 character until null
//-----------------------------------------------------
wstring mst::ReadUTF16
(
    FILE * _file
)
{
    wstring str;

    bool valid = false;
    while (ftell(_file) < (long)m_fileSize)
    {
        wchar_t symbol;
        fread(&symbol, 2, 1, _file);
        symbol = _byteswap_ushort(symbol);

        // Append wchat_t to wstring or exit
        if (symbol)
        {
            str += symbol;
        }
        else
        {
            valid = !str.empty();
            break;
        }
    }

    return valid ? str : wstring();
}

//-----------------------------------------------------
// Save a mst file
//-----------------------------------------------------
bool mst::Save
(
    string const & _fileName,
    string & _errorMsg
)
{
    if (!m_loaded)
    {
        _errorMsg = "File not loaded!";
        return false;
    }

    FILE* output;
    fopen_s(&output, _fileName.c_str(), "wb");

    // SKIPPED: File size, offset table address, offset table size

    // Write 1BBINA
    fseek(output, 0x16, SEEK_SET);
    WriteAscii(output, "1BBINA", false);

    // Write WTXT
    fseek(output, 0x20, SEEK_SET);
    WriteAscii(output, "WTXT", false);

    // SKIPPED: table name address

    unsigned int currentAddress = 0;
    unsigned int rootAddress = 0x20;

    // Write number of entries
    fseek(output, 0x28, SEEK_SET);
    WriteInt(output, m_entries.size());

    // SKIPPED: Entry name, subtitles and tags address
    struct EntryAddresses
    {
        unsigned int m_nameAddress;
        unsigned int m_subtitlesAddress;
        unsigned int m_tagsAddress;
    };
    vector<EntryAddresses> addresses(m_entries.size());

    // Write subtitles
    fseek(output, 0x0C * m_entries.size(), SEEK_CUR);
    for (unsigned int i = 0; i < m_entries.size(); ++i)
    {
        // Save subtitle address for this entry
        addresses[i].m_subtitlesAddress = ftell(output) - rootAddress;

        TextEntry const& entry = m_entries[i];
        for (unsigned int s = 0; s < entry.m_subtitles.size(); ++s)
        {
            wstring const& subtitle = entry.m_subtitles[s];

            if (s != entry.m_subtitles.size() - 1)
            {
                // Subtitle has more than one tab
                WriteUTF16(output, subtitle + L"\f", false);
            }
            else
            {
                // This is the last or only subtitle tab
                WriteUTF16(output, subtitle, true);
            }
        }
    }

    // Write table name and address
    currentAddress = ftell(output);
    fseek(output, 0x24, SEEK_SET);
    WriteInt(output, currentAddress - rootAddress);
    fseek(output, currentAddress, SEEK_SET);
    WriteAscii(output, m_tableName);

    // Write entry name and tags
    for (unsigned int i = 0; i < m_entries.size(); ++i)
    {
        TextEntry const& entry = m_entries[i];

        // Write and save name address for this entry
        addresses[i].m_nameAddress = ftell(output) - rootAddress;
        WriteAscii(output, entry.m_name);

        // Write and save tags address for this entry, can have no tags
        if (!entry.m_tags.empty())
        {
            addresses[i].m_tagsAddress = ftell(output) - rootAddress;
        }

        for (unsigned int t = 0; t < entry.m_tags.size(); ++t)
        {
            string const& tag = entry.m_tags[t];

            if (t != entry.m_tags.size() - 1)
            {
                // This entry has more than one tags
                WriteAscii(output, tag + ",", false);
            }
            else
            {
                // This is the last or only tag
                WriteAscii(output, tag, true);
            }
        }
    }

    // Go back and write offsets for all the entries
    currentAddress = ftell(output);
    fseek(output, 0x2C, SEEK_SET);
    for (EntryAddresses const& data : addresses)
    {
        WriteInt(output, data.m_nameAddress);
        WriteInt(output, data.m_subtitlesAddress);
        WriteInt(output, data.m_tagsAddress);
    }

    // Check for 4-byte alignment for offset table at the end of file
    unsigned int misalignment = currentAddress % 4;
    if (misalignment)
    {
        currentAddress = currentAddress + (4 - misalignment);
    }

    // Get offset table, always starts with "AB"
    // 'A': Skip "WTXT"
    // 'B': Skip table name offset and entry count
    string offsetTable = "AB";
    for (unsigned int i = 0; i < m_entries.size(); ++i)
    {
        TextEntry const& entry = m_entries[i];
        if (entry.m_tags.empty())
        {
            offsetTable += "AB";
        }
        else
        {
            offsetTable += "AAA";
        }
    }

    // Resize by -1 because last offset is not needed
    if (offsetTable.empty())
    {
        _errorMsg = "Empty offset table...?";
        return false;
    }
    else
    {
        offsetTable.resize(offsetTable.size() - 1);
    }

    // Write offset table address
    fseek(output, 0x04, SEEK_SET);
    WriteInt(output, currentAddress - rootAddress);

    // Write offset table content
    fseek(output, currentAddress, SEEK_SET);
    WriteAscii(output, offsetTable, false);

    // Offset table must end with 4-byte alignment
    misalignment = offsetTable.size() % 4;
    if (misalignment)
    {
        char buffer[4] = {'\0', '\0', '\0', '\0'};
        fwrite(buffer, 1, 4 - misalignment, output);
    }

    // Go back and write offset table size
    unsigned int offsetTableAddress = currentAddress;
    currentAddress = ftell(output);
    fseek(output, 0x08, SEEK_SET);
    WriteInt(output, currentAddress - offsetTableAddress);

    // Finally go back and write file size
    fseek(output, 0x00, SEEK_SET);
    WriteInt(output, currentAddress);
    m_fileSize = currentAddress;

    fclose(output);
    return true;
}

//-----------------------------------------------------
// Write 4 bytes from int
//-----------------------------------------------------
void mst::WriteInt
(
    FILE * _file,
    unsigned int _writeInt
)
{
    _writeInt = _byteswap_ulong(_writeInt);
    fwrite(&_writeInt, sizeof(unsigned int), 1, _file);
}

//-----------------------------------------------------
// Read a bytes from string
//-----------------------------------------------------
void mst::WriteAscii
(
    FILE * _file,
    string _writeString,
    bool _termination
)
{
    // Covert string to char*
    unsigned int stringLength = _writeString.size();
    char *stringBuffer = new char[stringLength + 2];
    strcpy_s(stringBuffer, stringLength + 1, _writeString.c_str());

    // Add null termination
    if (_termination)
    {
        stringBuffer[stringLength + 1] = stringBuffer[stringLength];
        stringBuffer[stringLength] = 0;
    }

    // Write bytes
    fwrite(stringBuffer, 1, stringLength + _termination, _file);
    delete[] stringBuffer;
}

//-----------------------------------------------------
// Read a bytes from UTF16 string
//-----------------------------------------------------
void mst::WriteUTF16
(
    FILE * _file,
    wstring _writeString,
    bool _termination
)
{
    // Covert wstring to wchar_t*
    unsigned int stringLength = _writeString.size();
    wchar_t *stringBuffer = new wchar_t[stringLength + 2];
    wcscpy_s(stringBuffer, stringLength + 1, _writeString.c_str());

    // Swap all byte pairs
    for (unsigned int i = 0; i < stringLength; ++i)
    {
        stringBuffer[i] = _byteswap_ushort(stringBuffer[i]);
    }

    // Add null termination
    if (_termination)
    {
        stringBuffer[stringLength + 1] = stringBuffer[stringLength];
        stringBuffer[stringLength] = 0;
    }

    // Write bytes
    fwrite(stringBuffer, sizeof(wchar_t), stringLength + _termination, _file);
    delete[] stringBuffer;
}

//-----------------------------------------------------
// Save a text file
//-----------------------------------------------------
void mst::Export
(
    string const & _fileName,
    bool _russian
)
{
    if (!m_loaded)
    {
        return;
    }

    FILE* output;
    fopen_s(&output, _fileName.c_str(), "w+,ccs=UTF-8");

    // Table Name
    wstring tableName;
    tableName.assign(m_tableName.begin(), m_tableName.end());
    fwprintf_s(output, L"Table Name: %s\n\n", tableName.c_str());

    for (size_t i = 0; i < m_entries.size(); ++i)
    {
        TextEntry const& entry = m_entries[i];

        wstring name;
        name.assign(entry.m_name.begin(), entry.m_name.end());
        fwprintf_s(output, L"-------------[%s]-------------\n", name.c_str());

        for (unsigned int s = 0; s < entry.m_subtitles.size(); ++s)
        {
            wstring subtitle = entry.m_subtitles[s];
            if (_russian)
            {
                for(wchar_t& chr : subtitle)
                {
                    if (m_unicodeToRussian.find(chr) != m_unicodeToRussian.end())
                    {
                        chr = m_unicodeToRussian[chr];
                    }
                }
            }
            fwprintf_s(output, L"%s\n\n", subtitle.c_str());
        }

        if (!entry.m_tags.empty())
        {
            fwprintf_s(output, L"Tags: ");
            for (size_t t = 0; t < entry.m_tags.size(); ++t)
            {
                string const& tag = entry.m_tags[t];
                wstring wtag;
                wtag.assign(tag.begin(), tag.end());
                fwprintf_s(output, L"%s", wtag.c_str());

                if (t != entry.m_tags.size() - 1)
                {
                    fwprintf_s(output, L", ");
                }
            }

            fwprintf_s(output, L"\n");
        }

        fwprintf_s(output, L"\n");
    }

    fclose(output);
}

//-----------------------------------------------------
// Search by name or tags
//-----------------------------------------------------
int mst::Search
(
    string const & _str,
    unsigned int _start
) const
{
    for (unsigned int i = _start; i < m_entries.size(); i++)
    {
        TextEntry const& entry = m_entries[i];

        // Search in name
        if (entry.m_name.find(_str.c_str()) != string::npos)
        {
            return (int)i;
        }

        // Search in tags
        for (string const& tag : entry.m_tags)
        {
            if (tag.find(_str.c_str()) != string::npos)
            {
                return (int)i;
            }
        }
    }

    return -1;
}

//-----------------------------------------------------
// Search from subtitle text
//-----------------------------------------------------
int mst::Search
(
    wstring const & _str,
    unsigned int _start
) const
{
    for (unsigned int i = _start; i < m_entries.size(); i++)
    {
        TextEntry const& entry = m_entries[i];

        // Search in subtitle
        for (wstring const& subtitle : entry.m_subtitles)
        {
            if (subtitle.find(_str.c_str()) != wstring::npos)
            {
                return (int)i;
            }
        }
    }

    return -1;
}

//-----------------------------------------------------
// Get all the currect text entries
//-----------------------------------------------------
void mst::GetAllEntries
(
    vector<TextEntry>& _textEntries
) const
{
    if (m_entries.empty()) return;

    _textEntries.clear();
    _textEntries.reserve(m_entries.size());
    for (TextEntry const& entry : m_entries)
    {
        _textEntries.push_back(entry);
    }
}

//-----------------------------------------------------
// Get a specific text entry
//-----------------------------------------------------
mst::TextEntry mst::GetEntry
(
    unsigned int _id
) const
{
    if (_id >= m_entries.size())
    {
        assert(false);
        return mst::TextEntry();
    }

    return m_entries[_id];
}

//-----------------------------------------------------
// Get a specific text entry with matching name
//-----------------------------------------------------
mst::TextEntry mst::GetEntry
(
    string const& _str
) const
{
    for (unsigned int i = 0; i < m_entries.size(); i++)
    {
        TextEntry const& entry = m_entries[i];

        // Search in name
        if (entry.m_name.find(_str.c_str()) != string::npos)
        {
            return entry;
        }
    }

    return TextEntry();
}

//-----------------------------------------------------
// Add a new dummy entry
//-----------------------------------------------------
int mst::AddNewEntry()
{
    TextEntry entry;
    entry.m_name = "DUMMY_NAME";
    entry.m_subtitles.push_back(L"DUMMY_SUBTITLE");
    m_entries.push_back(entry);
    return m_entries.size() - 1;
}

//-----------------------------------------------------
// Remove existing entry
//-----------------------------------------------------
void mst::RemoveEntry
(
    unsigned int _id
)
{
    if (_id >= m_entries.size()) return;
    m_entries.erase(m_entries.begin() + _id);
}

//-----------------------------------------------------
// Modify existing entry
//-----------------------------------------------------
void mst::ModifyEntry
(
    unsigned int _id,
    TextEntry const & _entry
)
{
    if (_id >= m_entries.size()) return;
    m_entries[_id] = _entry;
}
