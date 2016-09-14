#include <stdlib.h>
#include <grm.h>
#include <epm.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <tccore/tctype.h>
#include <epm_toolkit_tc_utils.h>
#include <base_utils/ResultCheck.hxx>
#include "../process_error.hxx"
#include "../misc.hxx"
#include "oc9_addObjects.hxx"

/*
 * 3.1. берет у продвигаемого объекта из целей значение атрибута item_id до первого дефиса или пробела;
 * 3.2. находит все ревизии изделий предприятия (Oc9_CompanyPartRevision), все ревизии конструкторских документов (Oc9_KDRevision),
 * изделия предприятия (Oc9_CompanyPart) и конструкторские документы (Oc9_KD), у которых item_id начинается с полученного в (3.1) значения;
 * 3.3. выбрасывает из целей продвигаемый объект;
 * 3.4. добавляет в цели объекты, найденные в (3.2).
 */

int oc9_addObjects(EPM_action_message_t msg){
	try {
			printf("%s\n", "+into oc9_addObjects");
			tag_t *attachments,
				*attachments_to_add,
				root_task,
				temp_type_t,
				Oc9_CompanyPartRevision_type_t,
				Oc9_CompanyPart_type_t,
				Oc9_KDRevision_type_t,
				Oc9_KD_type_t,
				query_t = NULL_TAG,
				*results_found;
			const int entryCount = 1;
			int *attachments_types,
				*attachments_types_to_add,
				attachments_count = 0,
				attachments_count_to_add = 0,
				resultCount = 0;
			char *temp_item_id,
				*item_id,
				*temp_id,
				**entryNames,
				**entryValues;
			bool isOc9_CompanyPartRevisionType,
				isOc9_CompanyPartType,
				isOc9_KDRevisionType,
				isOc9_KDType;
			ResultCheck erc;

			erc = TCTYPE_find_type("Oc9_CompanyPartRevision", NULL, &Oc9_CompanyPartRevision_type_t);
			erc = TCTYPE_find_type("Oc9_CompanyPart", NULL, &Oc9_CompanyPart_type_t);
			erc = TCTYPE_find_type("Oc9_KDRevision", NULL, &Oc9_KDRevision_type_t);
			erc = TCTYPE_find_type("Oc9_KD", NULL, &Oc9_KD_type_t);

			erc = EPM_ask_root_task(msg.task, &root_task);
			erc = EPM_ask_all_attachments(root_task, &attachments_count, &attachments, &attachments_types);
			printf("attachments count=%d\n", attachments_count);

			if(attachments_count>0){
				erc = AOM_ask_value_string(attachments[0], "item_id", &temp_item_id);
				item_id = getId(temp_item_id);
				TC_write_syslog("Searching for id:\n");
				TC_write_syslog(item_id);
				TC_write_syslog("\n");
				if(item_id!=NULL){
					MEM_free(temp_item_id);
					erc = QRY_find("IdealPLM_SearchObjects", &query_t);
					if (query_t != NULL_TAG) {
						printf("%s\n", "Found IdealPLM_SearchObjects");
						char entryNamesArray[entryCount][QRY_uid_name_size_c + 1] = { "item_id_object" };

						entryNames = (char**) MEM_alloc( entryCount * sizeof(*entryNames));
						for (int index = 0; index < entryCount; index++) {
							entryNames[index] = (char*) MEM_alloc( strlen(entryNamesArray[index] + 1));
							if (entryNames[index])
								strcpy(entryNames[index], entryNamesArray[index]);
						}

						entryValues = (char**) MEM_alloc(entryCount * sizeof(char *));
						entryValues[0] = (char *) MEM_alloc(strlen(item_id) + 2);
						strcpy(entryValues[0], item_id);
						strcat(entryValues[0], "*");

						printf("%s\n", "Executing query");
						TC_write_syslog("Executing query\n");
						TC_write_syslog(entryValues[0]);
						erc = QRY_execute(query_t, entryCount, entryNames, entryValues, &resultCount, &results_found);
						printf("Found %d results\n", resultCount);
					} else {
						printf("%s\n", "Not found IdealPLM_SearchObjects");
						MEM_free(temp_item_id);
						return ITK_ok;
					}
				} else {
					printf("%s\n", "Freeing temp_item_id...");
					MEM_free(temp_item_id);
					printf("%s\n", "Done...");
					return ITK_ok;
				}
			}
			if(resultCount==0){
				TC_write_syslog("Found nothing\n");
				printf("%s\n", "Found nothing!");
				printf("%s\n", "Freeing entry names and values...");
				MEM_free(entryNames[0]);
				MEM_free(entryNames);
				MEM_free(entryValues[0]);
				MEM_free(entryValues);
				printf("%s\n", "Done...");
				return ITK_ok;
			}
			TC_write_syslog("Found some results\n");

			attachments_to_add = (tag_t*) MEM_alloc(resultCount * sizeof(tag_t));
			attachments_types_to_add = (int*) MEM_alloc(resultCount * sizeof(int));
			for(int i = 0; i < resultCount; i++){
				erc = TCTYPE_ask_object_type(results_found[i], &temp_type_t);
				erc = TCTYPE_is_type_of(temp_type_t, Oc9_CompanyPartRevision_type_t, &isOc9_CompanyPartRevisionType);
				erc = TCTYPE_is_type_of(temp_type_t, Oc9_CompanyPart_type_t, &isOc9_CompanyPartType);
				erc = TCTYPE_is_type_of(temp_type_t, Oc9_KDRevision_type_t, &isOc9_KDRevisionType);
				erc = TCTYPE_is_type_of(temp_type_t, Oc9_KD_type_t, &isOc9_KDType);
				if(!(isOc9_CompanyPartRevisionType || isOc9_CompanyPartType || isOc9_KDRevisionType || isOc9_KDType)){
					printf("%s\n", ".wrong type");
					continue;
				}
				erc = AOM_ask_value_string(results_found[i], "item_id", &temp_id);
				if(strncmp(temp_id, item_id, strlen(item_id))==0){
					TC_write_syslog(".adding to attachments\n");
					printf("%s\n", ".adding to attachments");
					attachments_to_add[attachments_count_to_add] = results_found[i];
					attachments_types_to_add[attachments_count_to_add] = EPM_target_attachment;
					attachments_count_to_add++;
				}
				MEM_free(temp_id);
			}

			if(resultCount>0){
				TC_write_syslog("Freeeing found results...\n");
				printf("%s\n", "Freeing found results...");
				MEM_free(results_found);
				TC_write_syslog("Done...\n");
				printf("%s\n", "Done...");
				TC_write_syslog("Freeing entry names and values...\n");
				printf("%s\n", "Freeing entry names and values...");
				MEM_free(entryNames[0]);
				MEM_free(entryNames);
				MEM_free(entryValues[0]);
				MEM_free(entryValues);
				TC_write_syslog("Done...\n");
				printf("%s\n", "Done...");
			}

			TC_write_syslog("Removing old attachments...\n");
			printf("%s\n", "Removing old attachments");
			erc = EPM_remove_attachments(root_task, attachments_count, attachments);
			TC_write_syslog("Adding new attachments...\n");
			printf("%s\n", "Adding new attachments");
			erc = EPM_add_attachments(root_task, attachments_count_to_add, attachments_to_add, attachments_types_to_add);

			if(attachments_count>0){
				TC_write_syslog("Freeing attachments...\n");
				printf("%s\n", "Freeing attachments...");
				MEM_free(attachments);
				TC_write_syslog("Done...\n");
				printf("%s\n", "Done...");
				TC_write_syslog("Freeing attachment types...\n");
				printf("%s\n", "Freeing attachment types...");
				MEM_free(attachments_types);
				TC_write_syslog("Done...\n");
				printf("%s\n", "Done...");
			}

			if(attachments_count_to_add>0){
				TC_write_syslog("Freeing attachments to add...\n");
				printf("%s\n", "Freeing attachments to add...");
				MEM_free(attachments_to_add);
				TC_write_syslog("Done...\n");
				printf("%s\n", "Done...");
				TC_write_syslog("Freeing attachments types...\n");
				printf("%s\n", "Freeing attachments types...");
				MEM_free(attachments_types_to_add);
				TC_write_syslog("Done...\n");
				printf("%s\n", "Done...");
			}

			TC_write_syslog("Freeing item_id not...\n");
			printf("%s\n", "Freeing item_id...");
			//MEM_free(item_id);
			TC_write_syslog("Done...\n");
			printf("%s\n", "Done...");
	} catch (...) {
		return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}

