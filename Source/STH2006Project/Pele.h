#pragma once

#include "PathManager.h"

class Pele
{
public:
	enum PeleState
	{
		PS_Wait,
		PS_StandUp,
		PS_Follow,
		PS_SitDown,
	};

	struct PeleData
	{
		PathDataCollection m_path;
		PathFollowData m_followData;

		void* m_pObject;
		PeleState m_state;
		bool m_triggerFired;

		PeleData() { reset(); }
		void reset();
		void parsePath();
		void advance(float dt);

		// helper
		float* getFrameTime();
	};

	static void applyPatches();

	// Members
	static PeleData m_data;
};

