#pragma once
class Application
{
private:
	static std::string m_modDir;
	static float m_deltaTime;

public:
	static void applyPatches();

	// Delta time
	static void setDeltaTime(float dt) { m_deltaTime = dt; }
	static float getDeltaTime() { return m_deltaTime; }

	// Mod direction (include last '/')
	static void setModDir(std::string const& dir) { m_modDir = dir; }
	static std::string const getModDirString() { return m_modDir; }
	static std::wstring const getModDirWString() { return std::wstring(m_modDir.begin(), m_modDir.end()); }
};

