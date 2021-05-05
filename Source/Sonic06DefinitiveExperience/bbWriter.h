#pragma once

#include "Configuration.h"

using namespace std;

class bbWriter
{

public:
	bbWriter(string const& filePath);
	~bbWriter();

	static void applyModel(string& name, ModelType type);
	static void applyLanguage(string& name, LanguageType type);
	static void applyPhysics(string& name, bool physics, ModelType type);

	void addAR(string const& name, int count);
	void addName(string const& name);
	void addReplace(string const& name, string const& name2);

private:
	ofstream m_file;
};

