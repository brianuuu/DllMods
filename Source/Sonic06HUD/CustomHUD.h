#pragma once
class CustomHUD
{
public:
	static void applyPatches();

	// Utilities
	static void CreateScreen
	(
		Chao::CSD::RCPtr<Chao::CSD::CProject>& project,
		boost::shared_ptr<Sonic::CGameObjectCSD>& object,
		Hedgehog::Base::CStringSymbol const& in_RenderableCategory,
		bool updateAtPause,
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
		bool updateAtPause,
		Sonic::CGameObject* parent
	);
	static void SetScreenVisible
	(
		bool const visible,
		boost::shared_ptr<Sonic::CGameObjectCSD>& object
	);

	// Main gameplay
	static bool m_scoreEnabled;
	static float m_missionMaxTime;
	static void __fastcall CHudSonicStageRemoveCallback
	(
		Sonic::CGameObject* This, void*, 
		Sonic::CGameDocument* pGameDocument
	);

	// Pause
	static uint32_t m_cursorPos;
	static bool m_canRestart;
	static void __fastcall CPauseRemoveCallback
	(
		Sonic::CGameObject* This, void*,
		Sonic::CGameDocument* pGameDocument
	);
	static void CreatePauseScreen(uint32_t* This);
	static void OpenPauseScreen();
	static void ClosePauseScreen();
	static void RefreshPauseCursor();
};

