#pragma once

enum SetdataEnum
{
	Base,
	Normal,
	Break,
	Effect,
	Design,
	Sound,
	Fallsignal,
	Online,
	Plan_ly00,
	Plan_ly01,
	Plan_ly02,
	Plan_ly03,
	Plan_ly04,
	Plan_ly05,
	Plan_ly06,
	Plan_ly07,
	Plan_ly08,
	Plan_ly09,
	Design_ly00,
	Design_ly01,
	Design_ly02,
	Design_ly03,
	Design_ly04,
	Design_ly05,
	Design_ly06,
	Design_ly07,
	Design_ly08,
	Design_ly09,
};

class SetdataPatcher
{
public:
	static void applyPatches();
};

