/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Allow 1up and 10rings to be locked
//				 Require #SystemCommonItemboxLock.ar.00 and ItemItemboxLock.ar.00 injection
/*----------------------------------------------------------*/

#pragma once

class Itembox
{
	struct PositionStr
	{
		std::string x, y, z;
	};
	typedef std::map<uint32_t, PositionStr> ItemboxData;

public:
	static void applyPatches();
	static void playItemboxSfx();
	static void playItemboxVoice();
	static void playRainbowRingVoice();
	static tinyxml2::XMLError getInjectStr(char const* pData, uint32_t size, std::string& injectStr);
};

