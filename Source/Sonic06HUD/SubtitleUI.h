#pragma once

enum CaptionButtonType : uint32_t
{
	CBT_A = 0,
	CBT_B,
	CBT_X,
	CBT_Y,
	CBT_LB,
	CBT_RB,
	CBT_LT,
	CBT_RT,
	CBT_Start,
	CBT_Back,
	CBT_LStick,
	CBT_RStick,
	CBT_DPad,
};

struct Caption
{
	std::vector<std::string> m_captions;
	std::map<uint32_t, CaptionButtonType> m_buttons;
	float m_duration;
};

struct CaptionData
{
	uint32_t* m_owner;
	uint32_t* m_captionStart;
	bool m_isCutscene;
	float m_timer;
	bool m_bypassLoading;
	std::deque<Caption> m_captions;

	PDIRECT3DTEXTURE9 m_textbox;
	PDIRECT3DTEXTURE9 m_buttonA;
	PDIRECT3DTEXTURE9 m_buttonB;
	PDIRECT3DTEXTURE9 m_buttonX;
	PDIRECT3DTEXTURE9 m_buttonY;
	PDIRECT3DTEXTURE9 m_buttonLB;
	PDIRECT3DTEXTURE9 m_buttonLT;
	PDIRECT3DTEXTURE9 m_buttonRB;
	PDIRECT3DTEXTURE9 m_buttonRT;
	PDIRECT3DTEXTURE9 m_buttonStart;
	PDIRECT3DTEXTURE9 m_buttonBack;
	PDIRECT3DTEXTURE9 m_buttonLStick;
	PDIRECT3DTEXTURE9 m_buttonRStick;
	PDIRECT3DTEXTURE9 m_buttonDPad;

	bool init();
	void clear()
	{
		m_owner = nullptr;
		m_captionStart = nullptr;
		m_isCutscene = false;
		m_timer = 0.0f;
		m_bypassLoading = false;
		m_captions.clear();
	}

	~CaptionData()
	{
		if (m_textbox) m_textbox->Release();
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

	static std::map<uint32_t, wchar_t> m_fontDatabase;
	static bool initFontDatabase();

	static CaptionData m_captionData;
	static void __cdecl addCaptionImpl(uint32_t* owner, uint32_t* caption, float duration, bool isCutscene);
	static void draw();
	static void clearDraw();
	static float drawCaptions(Caption const& caption, float alpha, bool isShadow = false, bool isCutscene = false);
};

