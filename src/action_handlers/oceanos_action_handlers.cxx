#include <iostream>
#include <string>
#include <base_utils/ResultCheck.hxx>
#include <tc/tc.h>
#include <property/prop_msg.h>
#include <tccore/custom.h>
#include <tccore/method.h>
#include "../process_error.hxx"
#include "oceanos_action_handlers.hxx"
#include "oc9_setInvNo.hxx"
#include "oc9_addObjects.hxx"
#include "oc9_assignDesignation.hxx"
#include "oc9_add_to_targets_rev.hxx"

/**
 *
 */
int oceanos_idealplm_custom_register_action_handlers(int * decision, va_list args) {
	try {
		printf("\n");
		printf("REGISTRERING ACTION HANDLERS...");
		printf("\n");

		int ifail= ITK_ok;
		ResultCheck erc;
		*decision = ALL_CUSTOMIZATIONS;

		ifail = EPM_register_action_handler("Oc9-setInvNo", "", oc9_setInvNo);
		ifail = EPM_register_action_handler("Oc9-addObjects", "", oc9_addObjects);
		ifail = EPM_register_action_handler("Oc9-assignDesignation", "", oc9_assignDesignation);
		ifail = EPM_register_action_handler("Oc9-addRevsToTargets", "", oc9_add_to_targets_rev);

	} catch (...) {
		return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}
