#pragma once

#include "Utils/mst.h"

enum SubtitleButtonType : uint32_t
{
	SBT_A = 0,
	SBT_B,
	SBT_X,
	SBT_Y,
	SBT_LB,
	SBT_RB,
	SBT_LT,
	SBT_RT,
	SBT_Start,
	SBT_Back,
	SBT_LStick,
	SBT_RStick,
	SBT_DPad,
};

struct Subtitle
{
	std::vector<std::string> m_subtitles;
	std::map<uint32_t, SubtitleButtonType> m_buttons;
	float m_duration = 0.0f;
	uint32_t m_cueID = 0;
};

struct Caption
{
	std::vector<std::string> m_captions;
};

struct CaptionData
{
	// caption system for HUB world
	float m_frame;
	bool m_bypassLoading;
	bool m_isFadeIn;
	std::string m_speaker;
	std::deque<Caption> m_captions;

	int m_acceptDialogSize;
	int m_rejectDialogSize;
	bool m_acceptDialogShown;
	float m_yesNoColorTime;
	float m_yesNoArrowFrame;

	// subtitle system for Hint object
	float m_timer;
	size_t m_subtitleCount;
	std::deque<Subtitle> m_subtitles;

	IUnknown* m_textbox;
	IUnknown* m_acceptbox;
	IUnknown* m_arrow;

	IUnknown* m_buttonA;
	IUnknown* m_buttonB;
	IUnknown* m_buttonX;
	IUnknown* m_buttonY;
	IUnknown* m_buttonLB;
	IUnknown* m_buttonLT;
	IUnknown* m_buttonRB;
	IUnknown* m_buttonRT;
	IUnknown* m_buttonStart;
	IUnknown* m_buttonBack;
	IUnknown* m_buttonLStick;
	IUnknown* m_buttonRStick;
	IUnknown* m_buttonDPad;

	bool init();
	void clear()
	{
		m_frame = 0.0f;
		m_bypassLoading = false;
		m_isFadeIn = true;
		m_speaker.clear();
		m_captions.clear();

		m_acceptDialogSize = -1;
		m_rejectDialogSize = -1;
		m_acceptDialogShown = false;
		m_yesNoColorTime = 0.0f;
		m_yesNoArrowFrame = 0.0f;

		m_timer = 0.0f;
		m_subtitles.clear();
	}

	~CaptionData()
	{
		if (m_textbox) m_textbox->Release();
		if (m_acceptbox) m_acceptbox->Release();
		if (m_arrow) m_arrow->Release();
		if (m_buttonA) m_buttonA->Release();
		if (m_buttonB) m_buttonB->Release();
		if (m_buttonX) m_buttonX->Release();
		if (m_buttonY) m_buttonY->Release();
		if (m_buttonLB) m_buttonLB->Release();
		if (m_buttonLT) m_buttonLT->Release();
		if (m_buttonRB) m_buttonRB->Release();
		if (m_buttonRT) m_buttonRT->Release();
		if (m_buttonStart) m_buttonStart->Release();
		if (m_buttonBack) m_buttonBack->Release();
		if (m_buttonLStick) m_buttonLStick->Release();
		if (m_buttonRStick) m_buttonRStick->Release();
		if (m_buttonDPad) m_buttonDPad->Release();
	}
};

class SubtitleUI
{
public:
	static void applyPatches();

	static SharedPtrTypeless m_subtitleSfx;
	static CaptionData m_captionData;
	static bool isPlayingCaption() { return !m_captionData.m_captions.empty(); }

	static void addCaption(std::vector<std::string> const& captions, std::string const& speaker = "", int acceptDialogSize = -1, int rejectDialogSize = -1);
	static float addSubtitle(std::string const& name, std::string const& id);
	static float addSubtitle(mst::TextEntry const& entry, std::vector<float> const& durationOverrides = {});
	static SubtitleButtonType getButtonTypeFromTag(std::string const& tag);

	static void draw();
	static void clearDraw();
	static void drawCaptions(Caption const& caption, float alpha);
	static float drawSubtitle(Subtitle const& subtitle, float alpha);

	using DialogCallbackFunc = void(*)(void*, uint32_t);
	static std::set<DialogCallbackFunc> m_acceptCallbacks;
	static std::set<DialogCallbackFunc> m_finishCallbacks;
	static void addDialogAcceptCallback(DialogCallbackFunc);
	static void addDialogFinishCallback(DialogCallbackFunc);
};

