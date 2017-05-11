#include <stdlib.h>
#include <grm.h>
#include <epm.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <tccore/tctype.h>
#include <tccore/grm.h>
#include <dataset.h>
#include <epm_toolkit_tc_utils.h>
#include <base_utils/ResultCheck.hxx>
#include "../process_error.hxx"
#include "../misc.hxx"
#include "oc9_assignDesignation.hxx"

/*
 * 14.1. считывает атрибут oc8_Type формы Oc8_ClassESKD, которая находится в целях, получает 6-значный цифровой код;
14.2. сверяет полученный 6-значный цифровой код с идентификатором любого Item в целях. Если идентификатор содержит тот же код (например, идентификатор Item=НПРД.777777.001-05 ВЭ, а oc8_Type=777777), тогда ничего дальше не делать. Если другой, то идем дальше.
14.3. осуществляет поиск объектов, начинающихся со значения "НПРД."+<oc8_Type>+"."
например:
oc8_Type = «777777»
найдены:
НПРД.777777.001-01 СБ
НПРД.777777.001
НПРД.777777.002
НПРД.777777.004 ВЭ
14.4. определяет свободный индекс, значение которого находится в диапазоне от 001 до 9999 (здесь внимательнее с сортировкой);
например для перечня:
НПРД.777777.001-01 СБ
НПРД.777777.001
НПРД.777777.002
НПРД.777777.004 ВЭ
индекс=003
14.5. заменяет у всех изделий предприятия (Oc9_CompanyPart) и конструкторских документов (Oc9_KD) из целей кусок значения в item_id, полученный в (3.1), на значение "НПРД."+<oc8_Type>+"."+<индекс>;
14.6. после завершения (14.5) заменяет у всех Dataset в целях наименование на item_id ревизий объектов, с которыми они связаны через IMAN_specification;
14.7. после завершения (14.6) заменяет имена файлов именованных ссылок для всех Dataset из целей на имена, равные именам Dataset (расширение не трогать!).
 */

char* getFreeNum(char** array, int elements_count){
	printf("%s\n", "--getting free num");
	int i = 0;
	int index = 0;
	int currentNo = 1;
	int resultNo = -1;
	char* result = NULL;
	char* tempId = NULL;
	char* posOfDot = NULL;
	bool valueIsSet = false;
	int* numArray;
	if(elements_count>0){
		numArray = (int*) MEM_alloc(sizeof(int) * elements_count);
		for(i = 0; i < elements_count; i++){
			printf(".getting id=\n");
			tempId = getId(array[i]);
			printf(".got id=\n");
			if(tempId==NULL) continue;
			printf(".tempid=%s\n", tempId);
			posOfDot = strrchr(tempId, '.');
			if(posOfDot!=NULL){
			printf(".posOfDot=%s\n", posOfDot);
				numArray[index] = atoi(++posOfDot);
				index++;
			}
			//MEM_free(tempId);
		}
		if(index>0){
			printf("--sorting array\n");
			qsort(numArray, index, sizeof(int), cmpfunc);
			while(!valueIsSet){
				if(currentNo>9999) break;
				if (isvalueinarray(currentNo, numArray, index)) {
					printf("Already have this Num: %d\n", currentNo);
					currentNo++;
					continue;
				} else {
					printf("Found free value = %d\n", currentNo);
					valueIsSet = true;
				}
			}
		}
	}
	resultNo = currentNo;
	if(resultNo<1000){
		printf("--<1000\n");
		result = (char*) MEM_alloc(4*sizeof(char));
		sprintf(result, "%03i", resultNo);
	} else {
		printf("-->=1000\n");
		result = (char*) MEM_alloc(5*sizeof(char));
		sprintf(result, "%i", resultNo);
	}
	if(elements_count>0){
		MEM_free(numArray);
	}
	return result;
}

char* replaceItemId(char* where, char* what){
	char* result;
	char* pos = strpbrk(where, " -");
	if(pos!=NULL){
		result = (char*) MEM_alloc(sizeof(what) + sizeof(++pos) + 1);
		strcpy(result, what);
		strcat(result, pos);
	} else {
		result = (char*) MEM_alloc(sizeof(what)+1);
		strcpy(result, what);
	}
	return result;
}

char* getExtension(char* name){
	char* result = NULL;
	char* pos = strrchr(name, '.');
	if(pos!=NULL){
		result = (char*) MEM_alloc(strlen(pos)*sizeof(char) + 1);
		strcpy(result, pos);
	}
	return result;
}

int oc9_assignDesignation(EPM_action_message_t msg){
	try {
			printf("%s\n", "+into oc9_assignDesignation");
			tag_t *attachments,
				root_task,
				temp_type_t,
				parent_temp_type_t,
				Oc8_ClassESKD_type_t = NULLTAG,
				Oc9_CompanyPart_type_t = NULLTAG,
				Oc9_KD_type_t = NULLTAG,
				dataset_type_t = NULLTAG,
				iman_file_type_t = NULLTAG,
				IMAN_specification_rel_type_t = NULLTAG,
				IMAN_Rendering_rel_type_t = NULLTAG,
				*related_objs,
				query_t = NULL_TAG;
			int *attachments_types,
				attachments_count = 0,
				resultCount = 0;
			const int entryCount = 1;
			bool valueIsSet = false,
				is_Oc8_ClassESKD_type,
				is_Oc9_CompanyPartType,
				is_Oc9_KDType,
				is_DatasetType,
				is_ImanFileType;
			char *oc8_Type = NULL,
				*first_item_id = NULL,
				**entryNames,
				**entryValues,
				*entry_value,
				**item_ids,
				*temp_item_id,
				*newIndex= NULL,
				*temp_type_s = NULL;
			ResultCheck erc;
			auto_itk_mem_free<tag_t> results_found;

			erc = TCTYPE_find_type("Oc8_ClassESKD", NULL, &Oc8_ClassESKD_type_t);
			if(Oc8_ClassESKD_type_t==NULLTAG){
				printf("Cannot find type Oc8_ClassESKD");
				return ITK_ok;
			}
			erc = TCTYPE_find_type("Oc9_KD", NULL, &Oc9_KD_type_t);
			if(Oc9_KD_type_t==NULLTAG){
				printf("Cannot find type Oc9_KD");
				return ITK_ok;
			}
			erc = TCTYPE_find_type("Oc9_CompanyPart", NULL, &Oc9_CompanyPart_type_t);
			if(Oc9_CompanyPart_type_t==NULLTAG){
				printf("Cannot find type Oc9_CompanyPart");
				return ITK_ok;
			}
			erc = TCTYPE_find_type("Dataset", NULL, &dataset_type_t);
			if(dataset_type_t==NULLTAG){
				printf("Cannot find type dataset");
				return ITK_ok;
			}
			erc = GRM_find_relation_type("IMAN_specification", &IMAN_specification_rel_type_t);
			if(IMAN_specification_rel_type_t==NULLTAG){
				printf("Cannot find type IMAN_specification");
				return ITK_ok;
			}
			erc = GRM_find_relation_type("IMAN_Rendering", &IMAN_Rendering_rel_type_t);
			if(IMAN_Rendering_rel_type_t==NULLTAG){
				printf("Cannot find type IMAN_Rendering");
				return ITK_ok;
			}

			erc = TCTYPE_find_type("ImanFile", NULL, &iman_file_type_t);
			if(dataset_type_t==NULLTAG){
				printf("Cannot find type dataset");
				return ITK_ok;
			}

			erc = EPM_ask_root_task(msg.task, &root_task);
			erc = EPM_ask_all_attachments(root_task, &attachments_count, &attachments, &attachments_types);
			printf("attachments count=%d\n", attachments_count);

			for(int i = 0; i< attachments_count; i++){
				if(attachments_types[i] == EPM_target_attachment){
					printf("%s\n", ".asking type");
					erc = TCTYPE_ask_object_type(attachments[i], &temp_type_t);
					printf("%s\n", ".asking type name");
					erc = TCTYPE_ask_display_name(temp_type_t, &temp_type_s);
					if(temp_type_s!=NULL){
						printf("%s=%s\n", ".asking type name", temp_type_s);
						MEM_free(temp_type_s);
					}
					printf("%s\n", ".comparing types");
					erc = TCTYPE_is_type_of(Oc8_ClassESKD_type_t, temp_type_t, &is_Oc8_ClassESKD_type);
					if(is_Oc8_ClassESKD_type){
						printf("%s\n", ".is Oc8_ClassESKD type");
						if(oc8_Type==NULL){
							erc = AOM_ask_value_string(attachments[i], "oc8_Type", &oc8_Type);
							printf("oc8Type==%s\n", oc8_Type);
						}
					} else {
						printf("%s\n", ".is NOT Oc8_ClassESKD type");
						if(first_item_id==NULL) {
							erc = AOM_ask_value_string(attachments[i], "item_id", &first_item_id);
							printf("FirstItemId=%s\n", first_item_id);
						}
					}
				}
			}

			if(oc8_Type==NULL || first_item_id==NULL || strstr(first_item_id, oc8_Type)!=NULL) {
				printf("Trying to free oc8_Type\n");
				if(oc8_Type) MEM_free(oc8_Type);
				printf("Trying to free first_item_id\n");
				if(first_item_id) MEM_free(first_item_id);
				printf("MEM_free done!\n");
				return ITK_ok;
			}

			erc = QRY_find("IdealPLM_SearchObjects", &query_t);
			if (query_t != NULL_TAG) {
				printf("%s\n", "Found IdealPLM_SearchObjects");
				char entryNamesArray[entryCount][QRY_uid_name_size_c + 1] = { "item_id_object" };

				entryNames = (char**) MEM_alloc( entryCount * sizeof *entryNames);
				for (int index = 0; index < entryCount; index++) {
				entryNames[index] = (char*) MEM_alloc( strlen(entryNamesArray[index] + 1));
				if (entryNames[index])
					strcpy(entryNames[index], entryNamesArray[index]);
				}

				entry_value = (char*) MEM_alloc(((strlen("НПРД.") + strlen(".*") + strlen(oc8_Type)) * sizeof(char) + 1));
				strcpy(entry_value, "\0");
				strcat(entry_value, "НПРД.");
				strcat(entry_value, oc8_Type);
				strcat(entry_value, ".*");
				entryValues = (char**) MEM_alloc(entryCount * sizeof(char *));
				entryValues[0] = (char *) MEM_alloc(strlen(entry_value) + 1);
				strcpy(entryValues[0], entry_value);
				MEM_free(entry_value);

				printf("%s\n", "Executing query");
				erc = QRY_execute(query_t, entryCount, entryNames, entryValues, &resultCount, results_found.operator tag_t **());
				printf("Found %d results\n", resultCount);
				item_ids = (char**) MEM_alloc(sizeof(*item_ids)*resultCount);
				for(int i = 0; i < resultCount; i++){
					erc = AOM_ask_value_string(results_found.operator tag_t *()[i], "item_id", &temp_item_id);
					item_ids[i] = (char*) MEM_alloc(sizeof(char) * strlen(temp_item_id) + 1);
					strcpy(item_ids[i], temp_item_id);
					printf(".found=%s\n", temp_item_id);
				}

				newIndex = getFreeNum(item_ids, resultCount);
				if(newIndex!=NULL){
					printf("Found new Index = %s", newIndex);
					for(int i = 0; i< attachments_count; i++){
						if(attachments_types[i] == EPM_target_attachment){
							printf("%s\n", ".asking type");
							erc = TCTYPE_ask_object_type(attachments[i], &temp_type_t);
							printf("%s\n", ".comparing types");
							erc = TCTYPE_is_type_of(Oc9_CompanyPart_type_t, temp_type_t, &is_Oc9_CompanyPartType);
							erc = TCTYPE_is_type_of(Oc9_KD_type_t, temp_type_t, &is_Oc9_KDType);
							if(is_Oc9_CompanyPartType || is_Oc9_KDType){
								printf("%s\n", ".is Oc9 type");
								char* item_id;
								printf("%s\n", ".asking item_id");
								erc = AOM_ask_value_string(attachments[i], "item_id", &item_id);
								char* replacement = (char*) MEM_alloc((strlen("НПРД..") + strlen(newIndex) + strlen(oc8_Type))*sizeof(char)+1);
								strcpy(replacement, "НПРД.");
								strcat(replacement, oc8_Type);
								strcat(replacement, ".");
								strcat(replacement, newIndex);
								printf(".replacement=%s\n", replacement);
								char* after = replaceItemId(item_id, replacement);
								printf(".after=%s\n", after);
								MEM_free(item_id);
								MEM_free(replacement);
								printf("%s\n", ".locking");
								erc = AOM_refresh(attachments[i], TRUE);
								printf("%s\n", ".setting");
								erc = AOM_set_value_string(attachments[i], "item_id", after);
								printf("%s\n", ".saving");
								erc = AOM_save(attachments[i]);
								printf("%s\n", ".unlocking");
								erc = AOM_refresh(attachments[i], FALSE);
								printf("%s\n", ".freeing");
								MEM_free(after);
							}
						}
					}
					for(int i = 0; i< attachments_count; i++){
						if(attachments_types[i] == EPM_target_attachment){
							printf("%s\n", ".asking type");
							erc = TCTYPE_ask_object_type(attachments[i], &temp_type_t);
							printf("%s\n", ".comparing types");
							erc = TCTYPE_ask_parent_type(temp_type_t, &parent_temp_type_t);
							erc = TCTYPE_is_type_of(dataset_type_t, parent_temp_type_t, &is_DatasetType);
							if(is_DatasetType){
								printf("%s\n", ".is Dataset type");
								char* item_id;
								int count;
								tag_t* primary_objects;
								erc = GRM_list_primary_objects_only(attachments[i], IMAN_specification_rel_type_t, &count, &primary_objects);
								if(count>0){
									erc = AOM_ask_value_string(primary_objects[0], "item_id", &item_id);
									//printf("%s\n", ".locking");
									//erc = AOM_refresh(attachments[i], TRUE);
									printf("%s\n", ".setting");
									erc = AOM_set_value_string(attachments[i], "object_name", item_id);
									//printf("%s\n", ".saving");
									//erc = AOM_save(attachments[i]);
									//printf("%s\n", ".unlocking");
									//erc = AOM_refresh(attachments[i], FALSE);
									MEM_free(primary_objects);
									printf("%s\n", "Working with a dataset...");
									int namedrefs_count = 0;
									tag_t* namedrefs;
									char* name;
									erc = AE_ask_dataset_named_refs(attachments[i], &namedrefs_count, &namedrefs);
									for(int j = 0; j < namedrefs_count; j++){
										erc = TCTYPE_ask_object_type(namedrefs[j], &temp_type_t);
										erc = TCTYPE_is_type_of(iman_file_type_t, temp_type_t, &is_ImanFileType);
										if(is_ImanFileType){
											erc = AOM_ask_value_string(namedrefs[j], "original_file_name", &name);
											printf("Found name = %s\n", name);
											char* extension = getExtension(name);
											char* newName;
											if(extension!=NULL){
												newName = (char*) MEM_alloc((strlen(item_id)+strlen(extension))*sizeof(char) + 1);
												strcpy(newName, item_id);
												strcat(newName, extension);
												MEM_free(extension);
											} else {
												newName = (char*) MEM_alloc(strlen(item_id)*sizeof(char) + 1);
												strcpy(newName, item_id);
											}
											erc = AOM_refresh(namedrefs[j], TRUE);
											erc = IMF_set_original_file_name(namedrefs[j], newName);
											erc = AOM_save(namedrefs[j]);
											erc = AOM_refresh(namedrefs[j], FALSE);
											printf("Set name = %s\n", newName);
											MEM_free(name);
											MEM_free(newName);
										}
									}
									MEM_free(item_id);
									if(namedrefs_count>0){
										MEM_free(namedrefs);
									}
								}
								erc = GRM_list_primary_objects_only(attachments[i], IMAN_Rendering_rel_type_t, &count, &primary_objects);
								if(count>0){
									erc = AOM_ask_value_string(primary_objects[0], "item_id", &item_id);
									//printf("%s\n", ".locking");
									//erc = AOM_refresh(attachments[i], TRUE);
									printf("%s\n", ".setting");
									erc = AOM_set_value_string(attachments[i], "object_name", item_id);
									//printf("%s\n", ".saving");
									//erc = AOM_save(attachments[i]);
									//printf("%s\n", ".unlocking");
									//erc = AOM_refresh(attachments[i], FALSE);
									MEM_free(primary_objects);
									printf("%s\n", "Working with a dataset...");
									int namedrefs_count = 0;
									tag_t* namedrefs;
									char* name;
									erc = AE_ask_dataset_named_refs(attachments[i], &namedrefs_count, &namedrefs);
									for(int j = 0; j < namedrefs_count; j++){
										erc = TCTYPE_ask_object_type(namedrefs[j], &temp_type_t);
										erc = TCTYPE_is_type_of(iman_file_type_t, temp_type_t, &is_ImanFileType);
										if(is_ImanFileType){
											erc = AOM_ask_value_string(namedrefs[j], "original_file_name", &name);
											printf("Found name = %s\n", name);
											char* extension = getExtension(name);
											char* newName;
											if(extension!=NULL){
												newName = (char*) MEM_alloc((strlen(item_id)+strlen(extension))*sizeof(char) + 1);
												strcpy(newName, item_id);
												strcat(newName, extension);
												MEM_free(extension);
											} else {
												newName = (char*) MEM_alloc(strlen(item_id)*sizeof(char) + 1);
												strcpy(newName, item_id);
											}
											erc = AOM_refresh(namedrefs[j], TRUE);
											erc = IMF_set_original_file_name(namedrefs[j], newName);
											erc = AOM_save(namedrefs[j]);
											erc = AOM_refresh(namedrefs[j], FALSE);
											printf("Set name = %s\n", newName);
											MEM_free(name);
											MEM_free(newName);
										}
									}
									MEM_free(item_id);
									if(namedrefs_count>0){
										MEM_free(namedrefs);
									}
								}
							}
						}
					}
				}
			} else {
				printf("%s\n", "Cannot find query IdealPLM_SearchObjects");
			}
			if(attachments_count>0){
				MEM_free(attachments);
				MEM_free(attachments_types);
			}

	} catch (...) {
		return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}

