#include "bbWriter.h"

bbWriter::bbWriter(std::string const& filePath)
{
	m_file.open(filePath);
	m_file << "[Main]\nCommandCount=1\nCommand0=Add:AddFixed\n\n[AddFixed]\n";
}

bbWriter::~bbWriter()
{
	m_file.close();
}

void bbWriter::applyModel(string& name, ModelType type)
{
	switch (type)
	{
		case ModelType::Sonic: break;
		case ModelType::SonicElise: name += "_Elise";
	}
}

void bbWriter::applyLanguage(string& name, LanguageType type)
{
	switch (type)
	{
	case LanguageType::English: break;
	case LanguageType::Japanese: name += "_JPN";
	}
}

void bbWriter::applyPhysics(string& name, bool physics, ModelType type)
{
	// for affecting animation speed
	if (physics && type == ModelType::Sonic)
	{
		name += "_phy";
	}
}

void bbWriter::applyRun(string& name, bool run, ModelType type)
{
	// for running at result screen
	if (run && type == ModelType::Sonic)
	{
		name += "_run";
	}
}

void bbWriter::addAR(std::string const& name, int count)
{
	for (int i = 0; i < count; i++)
	{
		m_file << name << ".ar." << setw(2) << setfill('0') << i << "\n";
	}
	m_file << name << ".arl" << "\n";
}

void bbWriter::addName(std::string const& name)
{
	m_file << name << "\n";
}

void bbWriter::addReplace(std::string const& name, std::string const& name2)
{
	if (name == name2)
	{
		addName(name);
	}
	else
	{
		m_file << name << " = " << name2 << "\n";
	}
}
