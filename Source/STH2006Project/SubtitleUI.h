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

	PDIRECT3DTEXTURE9 m_textbox;

	bool init();
	void clear()
	{
		m_owner = nullptr;
		m_frame = 0.0f;
		m_bypassLoading = false;
		m_isFadeIn = true;
		m_speaker.clear();
		m_captions.clear();
	}

	~CaptionData()
	{
		if (m_textbox) m_textbox->Release();
	}
};

class SubtitleUI
{
public:
	static void applyPatches();

	static CaptionData m_captionData;
	static bool isPlayingCaption() { return !m_captionData.m_captions.empty(); }

	static void addCaption(std::vector<std::string> const& captions, std::string const& speaker = "");
	static void draw();
	static void clearDraw();
	static void drawCaptions(Caption const& caption, float alpha);
};

