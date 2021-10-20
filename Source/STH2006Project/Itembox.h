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

inline const wchar_t* getItemboxTextureName(ItemboxType type)
{
	switch (type)
	{
	case IT_5ring:	 return L"Item_5ring.dds";
	case IT_10ring:	 return L"Item_10ring.dds";
	case IT_20ring:	 return L"Item_20ring.dds";
	case IT_1up:	 return L"Item_1up.dds";
	case IT_invin:	 return L"Item_invin.dds";
	case IT_speed:	 return L"Item_speed.dds";
	case IT_gauge:	 return L"Item_gauge.dds";
	case IT_shield:	 return L"Item_shield.dds";
	case IT_fire:	 return L"Item_fire.dds";
	default: 		 return L"";
	}
}

struct ItemboxGUI
{
	ItemboxType m_type;
	float m_frame;
	float m_pos;
	PDIRECT3DTEXTURE9 m_texture;

	ItemboxGUI(ItemboxType type)
		: m_type(type)
		, m_frame(0.0f)
		, m_pos(0.5f)
		, m_texture(nullptr)
	{}

	~ItemboxGUI()
	{
		if (m_texture)
		{
			m_texture->Release();
		}
	}
};

class Itembox
{
public:
	static void applyPatches();
	static void playItemboxSfx();
	static void __fastcall playItemboxPfx(void* This);
	static tinyxml2::XMLError getInjectStr(char const* pData, uint32_t size, std::string& injectStr);

	// ImGui
	static bool m_using06HUD;
	static std::deque<ItemboxGUI> m_guiData;
	static void setUsing06HUD(bool enabled) { m_using06HUD = enabled; }
	static void addItemToGui(ItemboxType type);
	static void draw();
};

