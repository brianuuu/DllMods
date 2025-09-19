/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Allow 1up and 10rings to be locked
/*----------------------------------------------------------*/

#pragma once

class Itembox
{
public:
	static void applyPatches();
	static void playItemboxSfx();
	static void __fastcall playItemboxPfx(void* This);
};

