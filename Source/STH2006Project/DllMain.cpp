#include "DllMain.h"

HINSTANCE DllMain::handle;

BOOL WINAPI DllMain(_In_ HINSTANCE instance, _In_ DWORD reason, _In_ LPVOID reserved)
{
	DllMain::handle = instance;

	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}
