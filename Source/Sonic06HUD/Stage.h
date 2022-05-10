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
	static void CreateScreen
	(
		Chao::CSD::RCPtr<Chao::CSD::CProject>& project, 
		boost::shared_ptr<Sonic::CGameObjectCSD>& object, 
		Hedgehog::Base::CStringSymbol const& in_RenderableCategory,
		Sonic::CGameObject* parent = nullptr
	);
	static void KillScreen
	(
		boost::shared_ptr<Sonic::CGameObjectCSD>& object
	);
	static void ToggleScreen
	(
		Chao::CSD::RCPtr<Chao::CSD::CProject>& project,
		boost::shared_ptr<Sonic::CGameObjectCSD>& object,
		Hedgehog::Base::CStringSymbol const& in_RenderableCategory,
		Sonic::CGameObject* parent
	);
	static void SetScreenVisible
	(
		bool const visible, 
		boost::shared_ptr<Sonic::CGameObjectCSD>& object
	);
	static void __fastcall CHudSonicStageRemoveCallback(Sonic::CGameObject* This, void*, Sonic::CGameDocument* pGameDocument);
};

