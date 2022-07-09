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

	static bool m_missionAccept;
	static bool m_missionAsStage;
	static int getMissionDialog(std::vector<std::string>& captions, uint32_t stageID, std::string const& name, std::string* speaker = nullptr);
	static void startMissionCompleteDialog(bool success);
};

