#include <iostream>
#include <string>
#include <vector>
#include <stdarg.h>
#include <stdlib.h>

#include <base_utils/ResultCheck.hxx>
#include <tc/tc.h>
#include <tc/preferences.h>
#include <tccore/grm.h>
#include <tccore/method.h>
#include <tccore/tctype.h>
#include <tccore/aom_prop.h>
#include <sa/sa.h>
#include <tccore/aom.h>
#include <ug_va_copy.h>

#include "../misc.hxx"
#include "../process_error.hxx"
#include "oc9_get_document_type_value.hxx"

#define NUM_ENTRIES 1

using namespace std;

int getDocumentType(char**, int, const char*);
char* getSymbolPart(const char*);
bool debug = true;

/*
 * Считывается item_id Oc9_KD. С конца строки до первого пробела считывается код документа.
 *
 * Если кода документа нет, то ищем в TC Oc9_CompanyPart с атрибутом item_id = item_id Oс9_KD. (Имя запроса - idealplm_comppart, поиск по атрибуту oc9_Designation)
 * Если такого Oc9_CompanyPart нет, тогда oc9_DocumentType равен пусто.
 * Если есть и у Oc9_CompanyPart атрибут oc9_TypeOfPart равен Part, тогда oc9_DocumentType = "". Иначе oc9_DocumentType = Спецификация.
 * Если есть и у Oc9_CompanyPart атрибут oc9_TypeOfPart равен пусто, тогда oc9_DocumentType = Тип не определен.
 *
 * Если код документа есть, то ищем его значение в значении preferences (Имя настройки тимцентра - Oc9_DocumentTypes).
 * Если в preferences код документа есть или есть код документа, но без числового значения (например, в preferences есть СБ, но нет СБ1), тогда присваивать oc9_DocumentType значение из preferences.
 * Если кода документа в preferences нет, тогда oc9_DocumentType=Тип не определен.
 */

int oc9_get_document_type_value(METHOD_message_t *mess, va_list args) {
	try {
		va_list vargs;
		va_copy(vargs, args);
		tag_t prop_tag = va_arg(vargs, tag_t);
		char **rvalue = va_arg(vargs, char**);
		va_end(vargs);

		ResultCheck erc;
		char* result = NULL; //Don't forget to free it!

		char* item_id = NULL; //Don't forget to free it!
		string item_id_short;

		erc = AOM_ask_value_string(mess->object_tag, "item_id", &item_id);

		string item_id_string(item_id);
		size_t posOfSpace = item_id_string.find_last_of(" ");
		if (posOfSpace != string::npos) {
			int n_values = 0;
			char **values = NULL;
			PREF_initialize();
			erc = PREF_ask_char_values("Oc9_DocumentTypes", &n_values, &values);
			if (debug)
				printf("\t n_values: %d \n", n_values);

			if (n_values > 0) {
				char** shortTypes = (char**) MEM_alloc(n_values * sizeof(shortTypes));
				char** longTypes = (char**) MEM_alloc(n_values * sizeof(longTypes));
				char* splitedString;
				for (int ii = 0; ii < n_values; ii++) {
					if (debug)
						std::cout << values[ii] << std::endl;
					splitedString = strtok(values[ii], "=");
					shortTypes[ii] = (char*) MEM_alloc(256);
					strcpy(shortTypes[ii], splitedString);
					if (debug)
						std::cout << splitedString << "=" << shortTypes[ii] << std::endl;
					splitedString = strtok(NULL, "=");
					longTypes[ii] = (char*) MEM_alloc(256);
					strcpy(longTypes[ii], splitedString);
					if (debug)
						std::cout << splitedString << "=" << longTypes[ii] << std::endl;
				}

				MEM_free(values);
				values = NULL;

				item_id_short = item_id_string.substr(posOfSpace + 1,
						item_id_string.length());
				int pos = getDocumentType(shortTypes, n_values,
						item_id_short.c_str());
				if (debug)
					printf("Найдена позиция: %i\n", pos);
				if (pos != -1) {
					result = (char*) MEM_alloc(strlen(longTypes[pos]) + 1);
					strcpy(result, longTypes[pos]);
				} else {
					// Тип не определен
					result = (char*) MEM_alloc(strlen("Тип не определен") + 1);
					strcpy(result, "Тип не определен");
				}
				for(int i=0; i<n_values; i++){
					MEM_free(shortTypes[i]);
					MEM_free(longTypes[i]);
				}
				MEM_free(shortTypes);
				MEM_free(longTypes);
			} else {
				// Тип не определен
				result = (char*) MEM_alloc(strlen("Тип не определен") + 1);
				strcpy(result, "Тип не определен");
				MEM_free(values);
				values = NULL;
			}

		} else {
			int entryCount;
			char** entryNames;
			char** entryValues;
			tag_t query_t = NULL_TAG;
			int resultCount;
			char entryNamesArray[NUM_ENTRIES][QRY_uid_name_size_c + 1] = {
					"oc9_Designation" };

			entryNames = (char**) MEM_alloc( NUM_ENTRIES * sizeof *entryNames);
			for (int index = 0; index < NUM_ENTRIES; index++) {
				entryNames[index] = (char*) MEM_alloc(
						strlen(entryNamesArray[index] + 1));
				if (entryNames[index])
					strcpy(entryNames[index], entryNamesArray[index]);
			}

			entryValues = (char**) MEM_alloc(NUM_ENTRIES * sizeof(char *));
			entryValues[0] = (char *) MEM_alloc(strlen(item_id) + 1);
			strcpy(entryValues[0], item_id);

			auto_itk_mem_free<tag_t> results_found;
			erc = QRY_find("IdealPLM_CompPart", &query_t);				//TODO
			if (query_t != NULL_TAG) {
				erc = QRY_execute(query_t, NUM_ENTRIES, entryNames, entryValues,
						&resultCount, results_found.operator tag_t **());

				if (resultCount != 0) {
					// Берём первый попавшийся объект
					tag_t found_kit = results_found.operator tag_t *()[0];

					char* typeOfPart;
					if (found_kit != NULL_TAG) {
						erc = AOM_ask_value_string(found_kit, "oc9_TypeOfPart",
								&typeOfPart);
						if (strcmp("", typeOfPart) == 0) {
							result = (char*) MEM_alloc(
									strlen("Тип не определен") + 1);
							strcpy(result, "Тип не определен");
						} else if (strcmp("Деталь", typeOfPart) == 0) {
							result = (char*) MEM_alloc(
									strlen(" ") + 1);
							strcpy(result, " ");
						} else {
							result = (char*) MEM_alloc(
									strlen("Спецификация") + 1);
							strcpy(result, "Спецификация");
						}
					}
				}

			}
			MEM_free(entryNames[0]);
			MEM_free(entryNames);
			MEM_free(entryValues[0]);
			MEM_free(entryValues);
		}

		if (result) {
			*rvalue = (char*) MEM_alloc(strlen(result) + 1);
			strcpy(*rvalue, result);
			MEM_free(result);
		}
		if(item_id) MEM_free(item_id);

		return ITK_ok;

	} catch (...) {
		return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}

int getDocumentType(/*vector<char*> typesShort*/char** typesShort, int n_types,
		const char* idShort) {
	int result = -1;
	int pos = 0;
	char* symbolPart = getSymbolPart(idShort);
	if (debug)
		printf("SymbolPart=%s\n", symbolPart);
	for (int i = 0; i < n_types; i++) {
		if (debug) printf("Comparing TypesShort[i]=%s and IdShort=%s\n", (typesShort[i]), idShort);
		if (strcmp(typesShort[i], idShort) == 0 && (strlen(typesShort[i]) == strlen(idShort))) {
			result = pos;
			break;
		} else if (strcmp(typesShort[i], symbolPart) == 0
				&& (strlen(typesShort[i]) != strlen(idShort))) {
			result = pos;
		} else if (strcmp(typesShort[i], symbolPart) == 0
				&& (strlen(typesShort[i]) == strlen(idShort))) {
			result = pos;
			break;
		}
		pos++;
	}
	MEM_free(symbolPart);
	return result;
}

char* getSymbolPart(const char* type) {
	char* stringg = (char*) MEM_alloc(3);
	int character;
	unsigned int i;
	for (i = 0; i < strlen(type) && i < 3; i++) {
		character = type[i];
		if ((character >= 'А' && character <= 'Я')
				|| (character >= 'а' && character <= 'я')) {
			stringg[i] = character;
		} else {
			break;
		}
	}
	stringg[i] = '\0';
	return stringg;
}
