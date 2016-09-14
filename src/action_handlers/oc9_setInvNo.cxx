#include <stdlib.h>
#include <grm.h>
#include <epm.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <epm_toolkit_tc_utils.h>
#include <base_utils/ResultCheck.hxx>
#include "../process_error.hxx"
#include "../misc.hxx"
#include "oc9_setInvNo.hxx"

/*
 * Для объектов из целей проверять, что если атрибут Инвентарный номер не задан (oc9_InvNo),
 * то присваивать его начиная с 700010000 (в том числе освободившийся).
 * Атрибут oc9_InvNo будет создан на уровне Item.
 */

int oc9_setInvNo(EPM_action_message_t msg) {
	try {
		printf("%s\n", "+into oc9_setInvNo");
		tag_t *attachments, root_task, temp_type_t, *related_objs, query_t = NULL_TAG;
		int *attachments_types,
			attachments_count = 0,
			*invNos,
			oc9_InvNo = 0,
			entryCount = 0,
			resultCount = 0,
			i = 0,
			j = 0,
			currentNo = 700010000;
		bool valueIsSet = false;
		ResultCheck erc;
		auto_itk_mem_free<tag_t> results_found;

		erc = EPM_ask_root_task(msg.task, &root_task);
		erc = EPM_ask_all_attachments(root_task, &attachments_count, &attachments, &attachments_types);
		printf("attachments count=%d\n", attachments_count);
		erc = QRY_find("IdealPLM_FindInvNos", &query_t);
		if (query_t != NULL_TAG) {
			printf("%s\n", "Found query IdealPLM_FindInvNos");
			for (i = 0; i < attachments_count; i++) {
				if (attachments_types[i] == EPM_target_attachment) {
					erc = AOM_ask_value_int(attachments[i], "oc9_InvNo", &oc9_InvNo);
					printf("InvNo=%d\n", oc9_InvNo);
					if (oc9_InvNo == 0 ) {
						erc = QRY_execute(query_t, entryCount, NULL, NULL, &resultCount, results_found.operator tag_t **());
						if (resultCount != 0) {
							tag_t found_item;
							int invNo;
							invNos = (int*) MEM_alloc(sizeof(int)*resultCount);
							for(j = 0; j < resultCount; j++){
								found_item = results_found.operator tag_t *()[j];
								erc = AOM_ask_value_int(found_item, "oc9_InvNo", &invNo);
								printf("Found InvNo=%d\n", oc9_InvNo);
								invNos[j] = invNo;
							}
							printf("%s\n", "------------");
							qsort(invNos, resultCount, sizeof(int), cmpfunc);
							for (j = 0; j < resultCount; j++) {
								printf("%d\n", invNos[j]);
							}
							printf("%s\n", "------------");
							printf("%s\n", "results>0");
						} else {
							printf("%s\n", "results=0");
						}
						while(!valueIsSet){
							if (isvalueinarray(currentNo, invNos, resultCount)) {
								printf("%s\n", "Already have this InvNo =(");
								currentNo++;
								continue;
							} else {
								erc = AOM_refresh(attachments[i], 1);
								erc = AOM_load(attachments[i]);
								erc = AOM_set_value_int(attachments[i], "oc9_InvNo", currentNo);
								erc = AOM_save(attachments[i]);
								erc = AOM_refresh(attachments[i], 0);
								currentNo++;
								valueIsSet = true;
							}
						}
					if(invNos) MEM_free(invNos);
					}
				}
				valueIsSet = false;
			}
		} else {
			printf("%s\n", "Query IdealPLM_FindInvNos NOT FOUND!");
		}

		MEM_free(attachments);
		MEM_free(attachments_types);
	} catch (...) {
		return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}
