/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Allow 1up and 10rings to be locked
//				 Require #SystemCommonItemboxLock.ar.00 and ItemItemboxLock.ar.00 injection
/*----------------------------------------------------------*/

#pragma once

enum ItemboxType : uint32_t
{
	IT_5ring = 0,
	IT_10ring,
	IT_20ring,
	IT_1up,
	IT_invin,
	IT_speed,
	IT_gauge,
	IT_shield,
	IT_fire,

	IT_COUNT
};

struct ItemboxGUI
{
	ItemboxType m_type;
	float m_frame;
	float m_pos;

	ItemboxGUI(ItemboxType type)
		: m_type(type)
		, m_frame(0.0f)
		, m_pos(0.5f)
	{}
};

class Itembox
{
public:
	static void applyPatches();
	static void playItemboxSfx();
	static void __fastcall playItemboxPfx(void* This);

	// ImGui
	static std::deque<ItemboxGUI> m_guiData;
	static void addItemToGui(ItemboxType type);
	static void draw();
	static void clearDraw();

	// Textures
	static bool initTextures();
	static PDIRECT3DTEXTURE9 m_item_5ring;
	static PDIRECT3DTEXTURE9 m_item_10ring;
	static PDIRECT3DTEXTURE9 m_item_20ring;
	static PDIRECT3DTEXTURE9 m_item_1up;
	static PDIRECT3DTEXTURE9 m_item_invin;
	static PDIRECT3DTEXTURE9 m_item_speed;
	static PDIRECT3DTEXTURE9 m_item_gauge;
	static PDIRECT3DTEXTURE9 m_item_shield;
	static PDIRECT3DTEXTURE9 m_item_fire;
	~Itembox()
	{
		if (m_item_5ring)  m_item_5ring->Release();
		if (m_item_10ring) m_item_10ring->Release();
		if (m_item_20ring) m_item_20ring->Release();
		if (m_item_1up)	   m_item_1up->Release();
		if (m_item_invin)  m_item_invin->Release();
		if (m_item_speed)  m_item_speed->Release();
		if (m_item_gauge)  m_item_gauge->Release();
		if (m_item_shield) m_item_shield->Release();
		if (m_item_fire)   m_item_fire->Release();
	}
};

