#pragma once
class Application
{
private:
	static std::string m_modDir;
	static float m_deltaTime;
	static float m_hudDeltaTime;

public:
	static void applyPatches();

	// Delta time
	static void setDeltaTime(float dt) { m_deltaTime = dt; }
	static float getDeltaTime() { return m_deltaTime; }
	static void setHudDeltaTime(float dt) { m_hudDeltaTime = dt; }
	static float getHudDeltaTime() { return m_hudDeltaTime == 0.0f ? 0.0f : m_deltaTime; }

	// Mod direction (include last '/')
	static void setModDir(std::string const& dir) { m_modDir = dir; }
	static std::string const getModDirString() { return m_modDir; }
	static std::wstring const getModDirWString() { return std::wstring(m_modDir.begin(), m_modDir.end()); }
};

