#pragma once
class CustomHUD
{
public:
	// Must match API
	enum SonicGemType : int
	{
		SGT_Blue = 0,
		SGT_Red,
		SGT_Green,
		SGT_Purple,
		SGT_Sky,
		SGT_White,
		SGT_Yellow,

		SGT_COUNT,
	};

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
	static SonicGemType m_sonicGemType;
	static std::map<SonicGemType, bool> m_sonicGemEnabled;
	static void __fastcall CHudSonicStageRemoveCallback
	(
		Sonic::CGameObject* This, void*, 
		Sonic::CGameDocument* pGameDocument
	);
	static void ScrollSonicGem(bool toRight, bool ignoreNone);
	static void RestartSonicGem();
	static void PlayInfoHUD(bool intro, bool instantStart);

	// Pause
	static int m_cursorPos;
	static bool m_isPamPause;
	static bool m_canRestart;
	static void __fastcall CPauseRemoveCallback
	(
		Sonic::CGameObject* This, void*,
		Sonic::CGameDocument* pGameDocument
	);
	static void CreatePauseScreen(uint32_t* This, bool isPamPause);
	static void OpenPauseScreen();
	static void ClosePauseScreen();
	static void RefreshPauseCursor();

	// Yes & No Window
	static int m_yesNoCursorPos;
	static float m_yesNoColorTime;
	static std::string m_yesNoWindowText;
	static void __fastcall CWindowImplRemoveCallback
	(
		Sonic::CGameObject* This, void*,
		Sonic::CGameDocument* pGameDocument
	);
	static void RefreshYesNoCursor();

	// ImGui
	static bool IsYesNoWindowDrawing() { return !m_yesNoWindowText.empty(); }
	static void draw();
};

