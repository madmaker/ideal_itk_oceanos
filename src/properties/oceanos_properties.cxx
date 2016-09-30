#include <iostream>
#include <string>
#include <base_utils/ResultCheck.hxx>
#include <tc/tc.h>
#include <property/prop_msg.h>
#include <tccore/custom.h>
#include <tccore/method.h>
#include "../process_error.hxx"
#include "oceanos_properties.hxx"
#include "oc9_get_document_type_value.hxx"
#include "oc9_get_MaterialSEPart1_value.hxx"
#include "oc9_get_MaterialSEPart2_value.hxx"
#include "oc9_get_TypeOfPart_EN.hxx"

/**
 *
 */
int oceanos_custom_idealplm_register_properties(int * decision, va_list args) {
	try {
		printf("\n");
		printf("REGISTRERING PROPERTIES...");
		printf("\n");

		ResultCheck erc;

		*decision = ALL_CUSTOMIZATIONS;
		METHOD_id_t meth = { 0 };

		erc = METHOD_register_prop_method("Oc9_KD", "oc9_DocumentType",
				PROP_ask_value_string_msg, oc9_get_document_type_value, NULL, &meth);

		erc = METHOD_register_prop_method("Oc9_KDRevision", "oc9_MaterialSEPart1",
				PROP_ask_value_string_msg, oc9_get_MaterialSEPart1_value, NULL, &meth);

		erc = METHOD_register_prop_method("Oc9_KDRevision", "oc9_MaterialSEPart2",
				PROP_ask_value_string_msg, oc9_get_MaterialSEPart2_value, NULL, &meth);

		erc = METHOD_register_prop_method("Part", "oc9_TypeOfPart_EN",
				PROP_ask_value_string_msg, oc9_get_TypeOfPart_EN, NULL, &meth);


	} catch (...) {
		return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}
