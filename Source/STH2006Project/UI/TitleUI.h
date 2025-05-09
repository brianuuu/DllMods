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
	MS_Idle,
	MS_FadeIn,
	MS_Main,

	MS_TrialSelect,
	MS_ActTrial,
	MS_ModeSelect,
	MS_TownTrial,

	MS_Option,
	MS_OptionSaving,
	MS_OptionAudio,
	MS_OptionUI,
	MS_OptionVO,
	MS_OptionDialog,
	MS_OptionSubtitle,

	MS_ActWarningYesNo,
	//MS_TownWarningYesNo,
	MS_DeleteSaveYesNo,
	MS_ReturnTitleYesNo,
	MS_QuitYesNo,
	MS_QuitYes,

	MS_FadeOut,
	MS_FadeOutTitle,
	MS_FadeOutStage,
	MS_FadeOutMission,
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
	YNTT_ReturnTitle,
	YNTT_ReturnTitleJP,
	YNTT_NewGame,
	YNTT_NewGameJP,

	YNTT_COUNT,
};

enum OptionType : int
{
	OT_Audio,
	OT_VO,
	OT_Dialog,
	OT_Subtitle,

	OT_COUNT,

	// Disabled
	OT_UI,
};

enum MenuTextType : int
{
	MTT_NewGame,
	MTT_NewGameJP,
	MTT_Continue,
	MTT_ContinueJP,
	MTT_TrialSelect,
	MTT_TrialSelectJP,
	MTT_TrialSelectAll,
	MTT_TrialSelectAllJP,
	MTT_ActTrial,
	MTT_ActTrialJP,
	MTT_TownTrial,
	MTT_TownTrialJP,
	MTT_SelectStage,
	MTT_SelectStageJP,
	MTT_SelectMission,
	MTT_SelectMissionJP,
	MTT_Option,
	MTT_OptionJP,
	MTT_OptionAudio,
	MTT_OptionAudioJP,
	MTT_OptionVO,
	MTT_OptionVOJP,
	MTT_OptionDialogue,
	MTT_OptionDialogueJP,
	MTT_OptionSubtitle,
	MTT_OptionSubtitleJP,

	MTT_COUNT
};

struct TrialData
{
	// Common
	uint32_t m_stage;
	bool m_disableSilverMedal;
	bool m_forceNotPlayable;
	bool m_playable;
	bool m_hardModePlayable; // boss only
	std::string m_header;
	std::string m_stageID;

	// act trial
	std::string m_actName;

	// town trial
	std::string m_missionName;
	std::string m_missionNameJP;
};

class TitleUI
{
public:
	struct CursorData
	{
		int m_index;
		bool m_hidden;
	};

	struct StageData
	{
		size_t m_stage;
		std::string m_stageID;

		bool m_disableSilverMedal;
		bool m_isBoss;
		bool m_isMission;

		char m_bestTime[16];
		uint32_t m_bestScore;
		uint32_t m_bestRing;
		uint32_t m_silverMedalCount;

		StageData()
		{
			m_stage = -1;
			m_stageID = "";

			m_isBoss = false;

			sprintf(m_bestTime, "");
			m_bestScore = 0;
			m_bestRing = 0;
			m_silverMedalCount = 0;
		}
	};

	struct ReturnData
	{
		MenuState m_menuState;
		int m_cursor1Index;
		int m_cursor2Index;
		int m_stageCursorIndex;
		int m_missionCursorIndex;

		ReturnData()
		{
			m_menuState = MenuState::MS_Idle;
			m_cursor1Index = 0;
			m_cursor2Index = 0;
			m_stageCursorIndex = 0;
			m_missionCursorIndex = 0;
		}
	};

	static void applyPatches();

	static void populateTrialData();
	static void refreshTrialAvailability();

	static void enterModeSelect();
	static void populateStageData(size_t stage, std::string stageID, bool disableSilverMedal);
	static void cursorStageSelect(int index, bool isMission);
	static void cursorMission(int index);
	static void cursorStage(int index);
	static void cursorStageArrow(int index);

	static void cursorSelect(CursorData& data, Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, uint32_t soundCueID = 0xFFFFFFFF);
	static void cursorLoop(CursorData const& data, Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene);

	static void optionEnable(bool enable);
	static void optionSetIndex(int index);
	static void optionAudioSetIndex(int index);
	static void optionOnOffSetIndex(int index);
	static void optionCursorSetIndex(int index, bool out = false, int subIndex = -1, bool subOptionAudio = true);

	static void menuTitleSecondary(bool to2nd, size_t patternIndex = 0);
	static void menuTextLeft(Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, bool out);
	static void menuTextRight(Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, bool out);

	// Yes & No Window
	static std::map<YesNoTextType, std::string> m_yesNoText;
	static int m_yesNoCursorPos;
	static float m_yesNoColorTime;
	static std::string m_yesNoWindowText;
	static void EnableYesNoWindow(bool enabled, bool defaultYes = true, std::string const& text = "");
	static void SetYesNoCursor(int pos);
	static std::string const& GetYesNoText(YesNoTextType type);

	static std::map<MenuTextType, std::string> m_menuText;
	static void drawMenu();
	static void drawYesNoWindow();
	static void drawStageData();
};

