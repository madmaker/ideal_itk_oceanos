//@<COPYRIGHT>@
//==================================================
//Copyright $2015.
//Siemens Product Lifecycle Management Software Inc.
//All Rights Reserved.
//==================================================
//@<COPYRIGHT>@

/* 
 * @file 
 *
 *   This file contains the implementation for the Extension Pm8_DocRelAutoDeletion
 *
 */
#include <tccore/grm.h>
#include <tccore/method.h>
#include <tccore/tctype.h>
#include <tccore/item.h>

#include "oc9_DocRel_autodeletion.hxx"
#include <base_utils/ResultCheck.hxx>
#include "../process_error.hxx"
#include "../misc.hxx"

int oc9_DocRel_autodeletion(METHOD_message_t *msg, va_list args) {
	try {
		printf("%s\n", "+into oc9_DocRel_autodeletion");
		ResultCheck erc;
		tag_t primary_object = NULL_TAG;
		tag_t secondary_object = NULL_TAG;
		tag_t IMAN_specification_rel_type_t = NULLTAG;
		tag_t Oc9_KDRevision_type_t = NULLTAG;
		tag_t Oc9_CompanyPartRevision_type_t = NULLTAG;
		tag_t target_item;
		tag_t target_item_rev;
		char* primary_name;
		char* secondary_name;

		erc = GRM_ask_primary(msg->object_tag, &primary_object);
		erc = GRM_ask_secondary(msg->object_tag, &secondary_object);

		erc = GRM_find_relation_type("IMAN_specification",
				&IMAN_specification_rel_type_t);
		erc = TCTYPE_find_type("Oc9_KDRevision", NULL, &Oc9_KDRevision_type_t);
		erc = TCTYPE_find_type("Oc9_CompanyPartRevision", NULL,
				&Oc9_CompanyPartRevision_type_t);

		if (IMAN_specification_rel_type_t == NULLTAG
				|| Oc9_KDRevision_type_t == NULLTAG
				|| Oc9_CompanyPartRevision_type_t == NULLTAG)
			return ITK_ok;

		int num_rels = 0;
		auto_itk_mem_free<tag_t> source_rels;
		erc = GRM_list_primary_objects_only(primary_object,
				IMAN_specification_rel_type_t, &num_rels,
				source_rels.operator tag_t **());

		if (num_rels > 0) {
			target_item_rev = source_rels.operator tag_t *()[0];
		}

		if (target_item_rev != NULLTAG) {
			tag_t target_type_tag = NULL_TAG;
			erc = TCTYPE_ask_object_type(target_item_rev, &target_type_tag);
			//check if we get BO Item or it's descendant type
			logical target_has_valid_type = false;
			erc = TCTYPE_is_type_of(target_type_tag, Oc9_KDRevision_type_t,
					&target_has_valid_type);
			if (target_has_valid_type) {
				erc = ITEM_ask_item_of_rev(target_item_rev, &target_item);
			} else {
				return ITK_ok;
			}
		}

		char* doc_rel_name = "Oc9_DocRel";
		tag_t doc_rel_type = NULL_TAG;
		tag_t user_data = NULL_TAG;
		tag_t doc_rel_tag = NULL_TAG;
		tag_t rel_to_del = NULLTAG;
		erc = TCTYPE_find_type(doc_rel_name, NULL, &doc_rel_type);
		if (doc_rel_type == NULL_TAG) {
			printf("Oc9_DocRel TYPE NOT FOUND!!!");
			return ITK_ok;
		}
		erc = GRM_find_relation(secondary_object, target_item, doc_rel_type, &rel_to_del);
		if(rel_to_del!=NULLTAG){
			erc = GRM_delete_relation(rel_to_del);
		}
	} catch (...) {
		return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}
