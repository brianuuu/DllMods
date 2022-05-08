#include <memory>
#include "Controller.h"

ControllerInfo GetXncpNames(int buttonType, bool isFixExtended, bool isHideScore)
{
	switch (buttonType) {
		case PS3:
			return Dualshock3::getControllerInfo(isFixExtended, isHideScore);
		case SWITCH:
			return Switch::getControllerInfo(isFixExtended, isHideScore);
		case XBOX360:
			return Xbox360::getControllerInfo(isFixExtended, isHideScore);
		case XBOXSERIES:
			return XboxSeries::getControllerInfo(isFixExtended, isHideScore);
		default:
			return { };
	}
}