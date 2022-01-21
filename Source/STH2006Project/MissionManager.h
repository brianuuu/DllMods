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
	static std::string getMissionDialog(std::vector<std::string>& captions, uint32_t stageID, std::string const& name);
	static void startMissionCompleteDialog(bool success);
};

