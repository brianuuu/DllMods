#pragma once

enum CaptionButtonType : uint32_t
{
	CBT_A = 0,
	CBT_B,
	CBT_X,
	CBT_Y,
	CBT_LB,
	CBT_LT,
	CBT_RB,
	CBT_RT,
	CBT_Start,
	CBT_Back,
};

struct Caption
{
	std::string m_caption;
	float m_duration;
};

struct CaptionData
{
	uint32_t* m_owner;
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

	void init();
	void clear()
	{
		m_owner = nullptr;
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
	}
};

class Omochao
{
public:
	static void applyPatches();

	static CaptionData m_captionData;
	static void __cdecl addCaptionImpl(uint32_t* owner, uint32_t* caption, float duration);
	static void draw();
};

