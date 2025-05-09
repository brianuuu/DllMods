#pragma once
class WhosCaptain
{
public:
	enum CaptainState
	{
		CS_Idle,
		CS_Accept,
		CS_FindCaptain,
		CS_Success,
		CS_Fail,
	};

	static void* m_pCaptain;
	static CaptainState m_state;
	static void applyPatches();

	static void callbackCaptainAccept(void* pObject, uint32_t dialogID);
	static void callbackDialogFinish(void* pObject, uint32_t dialogID);
};

