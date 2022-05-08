/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Apply stage specific codes
/*----------------------------------------------------------*/

#pragma once

class Stage
{
public:
	static void applyPatches();

	static std::string m_lapTimeStr;
	static float m_checkpointTimer;
	static void draw();
	static void clearDraw();

	// Custom HUD
	static bool m_scoreEnabled;
	static float m_missionMaxTime;
	static void CreateScreen(Sonic::CGameObject* pParentGameObject);
	static void KillScreen();
	static void ToggleScreen(const bool visible, Sonic::CGameObject* pParentGameObject);
	static void __fastcall CHudSonicStageRemoveCallback(Sonic::CGameObject* This, void*, Sonic::CGameDocument* pGameDocument);
};

