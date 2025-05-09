#include "Configuration.h"
#include "DllMain.h"
#include "Resources.h"
#include "Window.h"

HICON __stdcall LoadSonicNextIcon(HINSTANCE hInstance, LPCSTR lpIconName)
{
	return LoadIconA(DllMain::handle, (LPCSTR)IDI_ICON);
}

void Window::applyPatches()
{
	if (Configuration::m_usingCustomWindow)
	{
		// Use custom game window title.
		const char* title = "SONIC THE HEDGEHOG";
		WRITE_STATIC_MEMORY(0x1606C50, title + '\0', strlen(title) + 1);

		// Use SONIC THE HEDGEHOG's icon for the window.
		WRITE_CALL(0xE7B843, &LoadSonicNextIcon);
		WRITE_NOP(0xE7B848, 1);
	}
}