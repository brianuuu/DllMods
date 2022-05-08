#pragma once

#include <string>

#define XBOX360 0
#define PS3 1
#define SWITCH 2
#define XBOXSERIES 3

/**
	Struct used to get the names of XNCP files used by the various controller types.
**/
typedef struct controller {
	int controllertype = XBOX360;

	const char* btn = 0;
	const char* win = 0;
	const char* pam = 0;
	const char* trick = 0;
	const char* how = 0;
	const char* bt = 0;

	const char* ui = 0;
} ControllerInfo;

/**
	Gets XNCP names based on controller type. This must be changed when adding new controllers.
**/
ControllerInfo GetXncpNames(int buttonType, bool isFixExtended, bool isHideScore);

/**
	Definitions for the various controllers. This must be changed when adding new controllers.
**/
namespace Dualshock3 {
	ControllerInfo getControllerInfo(bool isFixExtended, bool isHideScore);
}
namespace Switch {
	ControllerInfo getControllerInfo(bool isFixExtended, bool isHideScore);
}
namespace Xbox360 {
	ControllerInfo getControllerInfo(bool isFixExtended, bool isHideScore);
}
namespace XboxSeries {
	ControllerInfo getControllerInfo(bool isFixExtended, bool isHideScore);
}