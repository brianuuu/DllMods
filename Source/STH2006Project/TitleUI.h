#pragma once
enum TitleState : int
{
	TS_FadeIn,
	TS_Wait,
	TS_Confirm,
	TS_FadeOut,
	TS_End
};

enum MenuState : int
{
	MS_FadeIn,
	MS_Main,
	MS_TrialSelect,
	MS_Option,
};

enum MenuType : int
{
	MT_NewGame = 0,
	MT_Continue,
	MT_TrialSelect,
	MT_Option,
	MT_QuitGame,

	MT_COUNT,
};

enum TrialMenuType : int
{
	TMT_Act,
	TMT_Town,

	TMT_COUNT,
};

class TitleUI
{
public:
	struct CursorData
	{
		int m_index;
		bool m_hidden;
	};

	static void applyPatches();

	static void cursorSelect(CursorData& data, Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, std::vector<std::string> const& selectNames, uint32_t soundCueID = 0xFFFFFFFF);
	static void cursorLoop(CursorData const& data, Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, std::vector<std::string> const& loopNames);

	static void drawMenu();
};

