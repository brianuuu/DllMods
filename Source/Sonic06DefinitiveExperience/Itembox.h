/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Allow 1up and 10rings to be locked
//				 Require #SystemCommonItemboxLock.ar.00 and ItemItemboxLock.ar.00 injection
/*----------------------------------------------------------*/

#pragma once

class Itembox
{
public:
	static void applyPatches();
	static void playItemboxSfx();
	static void __fastcall playItemboxPfx(void* This);
};

