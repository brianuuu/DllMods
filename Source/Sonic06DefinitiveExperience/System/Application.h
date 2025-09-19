#pragma once
class Application
{
public:
	static void applyPatches();
	static float getDeltaTime();

	static Sonic::EKeyState keyHoldState;
	static Sonic::EKeyState keyReleasedState;
	static bool getKeyIsReleased(const Sonic::EKeyState key);
};

