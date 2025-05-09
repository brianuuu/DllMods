/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Animate Solenna NPCs to follow paths
/*----------------------------------------------------------*/

#pragma once
#include "Managers/PathManager.h"

class SoleannaNPC
{
public:
	static void applyPatches();

	// Members
	static PathDataCollection m_pathsMapA;
	static PathDataCollection m_pathsMapB;
};

