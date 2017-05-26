#include <stdio.h>
#include <base_utils/ResultCheck.hxx>
#include <tc/tc.h>
#include <tccore/custom.h>
#include "process_error.hxx"
#include "oceanos_custom_idealplm_exits.hxx"
#include "properties/oceanos_properties.hxx"
#include "actions/oceanos_actions.hxx"
#include "action_handlers/oceanos_action_handlers.hxx"

//cppcheck-suppress unusedFunction
extern "C" __declspec (dllexport) int oceanos_custom_idealplm_register_callbacks() {
	try {
		printf("\n---=Loading custom library=---\n");
		ResultCheck erc;

		erc = CUSTOM_register_exit("oceanos_custom_idealplm", "USER_register_properties",
				oceanos_custom_idealplm_register_properties);

		erc = CUSTOM_register_exit("oceanos_custom_idealplm", "USER_init_module",
				(CUSTOM_EXIT_ftn_t) oceanos_idealplm_custom_register_actions);

		erc = CUSTOM_register_exit("oceanos_custom_idealplm", "USER_gs_shell_init_module",
				oceanos_idealplm_custom_register_action_handlers);

	} catch (...) {
		return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}
