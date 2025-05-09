/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2022
//	Description: 
/*----------------------------------------------------------*/

#pragma once

class MissionManager
{
public:
	static void applyPatches();

	static void setMissionFailed();

	static bool m_missionAccept;
	static bool m_missionAsStage;
	static int getMissionDialog(std::vector<std::string>& captions, std::string const& section, std::string const& name, std::string* speaker = nullptr);
	static void startMissionCompleteDialog(bool success);

	static uint32_t m_genericNPCDialog;
	static void* m_genericNPCObject;
	static void startGenericDialog(std::string const& section, bool hasYesNo = false);
};

