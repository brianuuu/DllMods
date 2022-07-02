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
	MS_QuitYesNo,
	MS_QuitYes,
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

enum YesNoTextType : int
{
	YNTT_QuitGame,
	YNTT_QuitGameJP,

	YNTT_COUNT,
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

	static void cursorSelect(CursorData& data, Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, uint32_t soundCueID = 0xFFFFFFFF);
	static void cursorLoop(CursorData const& data, Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene);

	static void menuTextLeft(Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, bool out);
	static void menuTextRight(Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, bool out);

	// Yes & No Window
	static std::map<YesNoTextType, std::string> m_yesNoText;
	static int m_yesNoCursorPos;
	static float m_yesNoColorTime;
	static std::string m_yesNoWindowText;
	static void EnableYesNoWindow(bool enabled, std::string const& text = "");
	static void SetYesNoCursor(int pos);
	static std::string const& GetYesNoText(YesNoTextType type);

	static void drawMenu();
	static void drawYesNoWindow();
};

