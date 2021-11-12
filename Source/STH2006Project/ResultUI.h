/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Draws 06 style result UI using ImGUI
/*----------------------------------------------------------*/
#pragma once
#include "ScoreManager.h"

class ResultUI
{
private:
	enum ResultTextType : uint32_t
	{
		RTT_Score = 0,
		RTT_Time,
		RTT_Rings,
		RTT_TimeBonus,
		RTT_RingBonus,
		RTT_TotalScore,
		RTT_Rank,
		RTT_COUNT
	};

	struct ResultUIBox
	{
		ResultTextType m_type;
		bool m_started; // Move box into view
		ImVec2 m_finalPos;
		float m_posX; // current x-pos
		std::string m_text;
		float m_alpha;

		// static members
		PDIRECT3DTEXTURE9* m_texture;
		ImVec2 m_textureSize;
		ImVec2 m_textOffset;

		ResultUIBox() {}
		ResultUIBox(ResultTextType type, ImVec2 pos);
		void reset();
		void draw(float scaleX, float scaleY);
	};

	enum ResultState : uint32_t
	{
		RS_Idle = 0,
		RS_Border,
		RS_RightBox,
		RS_Score,
		RS_Time,
		RS_Rings,
		RS_TimeBonus,
		RS_RingBonus,
		RS_TimeBounsCount,
		RS_RingBonusCount,
		RS_TotalScore,
		RS_TotalScoreCount,
		RS_Rank,
		RS_RankShow,
		RS_FadeOut,
		RS_Finish
	};

	struct ResultUIData
	{
		ResultState m_state;
		float m_frame;
		int m_scoreDestination;
		int m_scoreCount;
		ResultUIBox m_boxes[ResultTextType::RTT_COUNT];

		ResultUIData();
		void reset();
		void start()
		{
			reset();
			nextState();
		}
		void draw();
		void nextState();
		void countScore(ResultTextType type);
	};

public:
	static float m_currentTime;
	static ResultData* m_resultData;
	static void applyPatches();

	// ImGui
	static ResultUIData m_resultUIData;
	static void draw();

	// Textures
	static bool m_init;
	static bool initTextures();
	static PDIRECT3DTEXTURE9 m_resultRankTextures[5];
	static PDIRECT3DTEXTURE9 m_resultNumTextures[10];
	static PDIRECT3DTEXTURE9 m_resultTextTextures[RTT_COUNT];
	static PDIRECT3DTEXTURE9 m_resultCommaTexture;
	static PDIRECT3DTEXTURE9 m_resultBoxTexture;
	static PDIRECT3DTEXTURE9 m_resultTotalBoxTexture;
	static PDIRECT3DTEXTURE9 m_resultScoreBoxTexture;
	static PDIRECT3DTEXTURE9 m_resultHeaderBoxTexture;
	static PDIRECT3DTEXTURE9 m_resultHeaderShadowTexture;
	static PDIRECT3DTEXTURE9 m_resultHeaderTextTexture;
	static PDIRECT3DTEXTURE9 m_resultRankBoxTexture;
	static PDIRECT3DTEXTURE9 m_resultFadeTexture;
	~ResultUI()
	{
		for (int i = 0; i < 5; i++)
		{
			if (m_resultRankTextures[i]) m_resultRankTextures[i]->Release();
		}

		for (int i = 0; i < 10; i++)
		{
			if (m_resultNumTextures[i]) m_resultNumTextures[i]->Release();
		}

		for (int i = 0; i < RTT_COUNT; i++)
		{
			if (m_resultTextTextures[i]) m_resultTextTextures[i]->Release();
		}

		if (m_resultCommaTexture)		m_resultCommaTexture->Release();
		if (m_resultBoxTexture)			m_resultBoxTexture->Release();
		if (m_resultTotalBoxTexture)	m_resultTotalBoxTexture->Release();
		if (m_resultScoreBoxTexture)	m_resultScoreBoxTexture->Release();
		if (m_resultHeaderBoxTexture)	m_resultHeaderBoxTexture->Release();
		if (m_resultHeaderShadowTexture)m_resultHeaderShadowTexture->Release();
		if (m_resultHeaderTextTexture)	m_resultHeaderTextTexture->Release();
		if (m_resultRankBoxTexture)		m_resultRankBoxTexture->Release();
		if (m_resultFadeTexture)		m_resultFadeTexture->Release();
	}

};

