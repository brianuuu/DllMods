/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Animate Solenna NPCs to follow paths
/*----------------------------------------------------------*/

#pragma once
#include "PathManager.h"

class SoleannaNPC
{
public:
	static void applyPatches();

	// Members
	static PathDataCollection m_pathsMapA;
	static PathDataCollection m_pathsMapB;
	static std::map<void*, PathFollowData> m_NPCs;
	static std::set<void*> m_pObjectsMapA;
	static std::set<void*> m_pObjectsMapB;

	// The Box
	static std::map<void*, float> m_pBoxes;
};

