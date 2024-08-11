/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2024
//	Description: Replicate Sonic 06 Physics for Shadow
/*----------------------------------------------------------*/

#include "NextGenPhysics.h"

#pragma once
class NextGenShadow
{
public:
	static void setAnimationSpeed_Shadow(NextGenAnimation& data);
	static void applyPatches();

	// X Action
	static float m_xHeldTimer;
	static bool bActionHandlerImpl();
};

