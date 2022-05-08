#include "Controller.h"

namespace Switch {
	ControllerInfo getControllerInfo(bool isFixExtended, bool isHideScore) {
		ControllerInfo toReturn;

		toReturn.controllertype = SWITCH;
		toReturn.btn = "ui_gp_btn_nswit";
		toReturn.win = "ui_nswdow";
		toReturn.pam = "ui_nsm";
		toReturn.trick = "ui_gp_nswit";
		toReturn.how = "ui_howns";
		toReturn.bt = "ui_btn_nside";

		if (isFixExtended)
			toReturn.ui = isHideScore ? "ui_gamensay" : "ui_gamensaa";
		else
			toReturn.ui = isHideScore ? "ui_gamensab" : "ui_gamensay";

		return toReturn;
	}
}