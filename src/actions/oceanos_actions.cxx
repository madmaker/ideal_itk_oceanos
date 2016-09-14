#include <iostream>
#include <string>
#include <base_utils/ResultCheck.hxx>
#include <tc/tc.h>
#include <property/prop_msg.h>
#include <tccore/custom.h>
#include <tccore/method.h>
#include "../process_error.hxx"
#include "oceanos_actions.hxx"
#include "oc9_DocRel_autocreation.hxx"
#include "oc9_DocRel_autodeletion.hxx"

/**
 *
 */
int oceanos_idealplm_custom_register_actions(int * decision, va_list args) {
	try {
		printf("\n");
		printf("REGISTRERING ACTIONS...");
		printf("\n");

		ResultCheck erc;
		*decision = ALL_CUSTOMIZATIONS;

		METHOD_id_t docrelautocreation_method;
		METHOD_id_t docrelautodeletion_method;

		// DocRel auto creation
		erc = METHOD_find_method("SE Interpart Draft Links", "createPost#Teamcenter::CreateInput,*", &docrelautocreation_method);

		if (docrelautocreation_method.id != NULLTAG) {
			erc = METHOD_add_action(docrelautocreation_method,
					METHOD_post_action_type,
					(METHOD_function_t) oc9_DocRel_autocreation, NULL);
		} else
			fprintf(stdout, "Method NOT found! (createPost#Teamcenter::CreateInput,*)\n", erc);

		// DocRel auto deletion
		erc = METHOD_find_method("SE Interpart Draft Links", "IMAN_delete", &docrelautodeletion_method);

		if (docrelautodeletion_method.id != NULLTAG) {
			erc = METHOD_add_action(docrelautodeletion_method,
					METHOD_pre_action_type,
					(METHOD_function_t) oc9_DocRel_autodeletion, NULL);
		} else
			fprintf(stdout, "Method NOT found! (docrelaautodeletion)\n", erc);

	} catch (...) {
		return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}
