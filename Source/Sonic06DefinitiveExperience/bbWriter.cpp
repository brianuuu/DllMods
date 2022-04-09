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

void bbWriter::applyModel(string& name, Configuration::ModelType type)
{
	switch (type)
	{
		case Configuration::ModelType::Sonic:		name = "06Sonic\\" + name; return;
		case Configuration::ModelType::SonicElise:	name = "06Elise\\" + name; return;
		case Configuration::ModelType::Blaze:		name = "06Blaze\\" + name; return;
	}
}
/*
void bbWriter::applyLanguage(string& name, Configuration::LanguageType type)
{
	switch (type)
	{
	case Configuration::LanguageType::English: break;
	case Configuration::LanguageType::Japanese: name += "_JPN";
	}
}

void bbWriter::applyPhysics(string& name, bool physics, Configuration::ModelType type)
{
	// for affecting animation speed
	if (physics && type == Configuration::ModelType::Sonic)
	{
		name += "_phy";
	}
}
*/
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
