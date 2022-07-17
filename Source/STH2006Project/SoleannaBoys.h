#pragma once
#include "PathManager.h"

class SoleannaBoys
{
public:
	static PathDataCollection m_paths;
	struct BoyData
	{
		void* m_pObjectRunning;
		bool m_stopped;
		PathFollowData m_followData;

		BoyData()
		{
			m_pObjectRunning = nullptr;
			m_stopped = false;
		}
	};
	static std::map<void*, BoyData> m_boys;
	static std::deque<void*> m_omochaos;

	static void applyPatches();

};

