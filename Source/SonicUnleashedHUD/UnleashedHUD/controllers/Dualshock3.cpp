#include "Controller.h"

namespace Dualshock3 {
	ControllerInfo getControllerInfo(bool isFixExtended, bool isHideScore) {
		ControllerInfo toReturn;

		toReturn.controllertype = PS3;
		toReturn.btn = "ui_gp_btn_pside";
		toReturn.win = "ui_psndow";
		toReturn.pam = "ui_psm";
		toReturn.trick = "ui_gp_psick";
		toReturn.how = "ui_howps";
		toReturn.bt = "ui_btn_pside";

		if (isFixExtended)
			toReturn.ui = isHideScore ? "ui_gamepsac" : "ui_gamepsaa";
		else
			toReturn.ui = isHideScore ? "ui_gamepsab" : "ui_gamepsay";

		return toReturn;
	}
}