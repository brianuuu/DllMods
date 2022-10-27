#pragma once

enum ItemboxType : uint32_t
{
	IT_10ring,
	IT_1up,

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

class ItemboxUI
{
private:
	static void applyPatches();

public:
	// ImGui
	static std::deque<ItemboxGUI> m_guiData;
	static void addItemToGui(ItemboxType type);
	static void draw();
	static void clearDraw();

	// Textures
	static bool initTextures();
	static IUnknown* m_item_10ring;
	static IUnknown* m_item_1up;
	~ItemboxUI()
	{
		if (m_item_10ring) m_item_10ring->Release();
		if (m_item_1up)	   m_item_1up->Release();
	}
};

