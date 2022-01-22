#pragma once

struct Caption
{
	std::vector<std::string> m_captions;
};

struct CaptionData
{
	uint32_t* m_owner;
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

	PDIRECT3DTEXTURE9 m_textbox;
	PDIRECT3DTEXTURE9 m_acceptbox;
	PDIRECT3DTEXTURE9 m_arrow;

	bool init();
	void clear()
	{
		m_owner = nullptr;
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
	}

	~CaptionData()
	{
		if (m_textbox) m_textbox->Release();
		if (m_acceptbox) m_acceptbox->Release();
		if (m_arrow) m_arrow->Release();
	}
};

class SubtitleUI
{
public:
	static void applyPatches();

	static CaptionData m_captionData;
	static bool isPlayingCaption() { return !m_captionData.m_captions.empty(); }

	static void addCaption(std::vector<std::string> const& captions, std::string const& speaker = "", int acceptDialogSize = -1, int rejectDialogSize = -1);
	static void draw();
	static void clearDraw();
	static void drawCaptions(Caption const& caption, float alpha);
};

