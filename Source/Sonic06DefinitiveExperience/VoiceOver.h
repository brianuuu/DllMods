/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Handle voices modification, add voices to 
//				 various gameplay (homing attack, rainbow ring etc.)
/*----------------------------------------------------------*/

#pragma once
class VoiceOver
{
public:
	static void applyPatches();

	// All public voices functions
	static void playItemboxVoice();
	static void playRainbowRingVoice();
};

