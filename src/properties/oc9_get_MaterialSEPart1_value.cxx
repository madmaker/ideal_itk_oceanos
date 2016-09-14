#include <base_utils/ResultCheck.hxx>
#include <tc/tc.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <tccore/tctype.h>
#include <tc/preferences.h>
#include <ug_va_copy.h>

#include "../misc.hxx"
#include "../process_error.hxx"
#include "oc9_get_MaterialSEPart1_value.hxx"

/*
 * Если oc9_StockCompanyPart не пуст, тогда:
oc9_MaterialSEPart2 = "Изделие-заготовка%RT"+<oc9_StockCompanyPart>
oc9_MaterialSEPart1 = ""

Если oc9_StockCommercialPart не пуст, тогда:
oc9_MaterialSEPart2 = "Изделие-заготовка%RT"+<oc9_ StockCommercialPart>
oc9_MaterialSEPart1 = ""

Если oc9_CADMaterial не пуст, а oc9_StockCompanyPart и oc9_StockCommercialPart пусты, тогда:
выполнить поиск Mat1Material с object_name= oc9_CADMaterial.

Если не найден, тогда:
oc9_MaterialSEPart1 = "Неверно задан материал"
oc9_MaterialSEPart2 = ""

Если найден:
1.       oc9_MaterialSEPart1=<oc9_CADMaterial>

2.       Если oc9_CADMaterial содержит в начале строки значение из преф. Oc9_SortamentTypes, тогда oc9_CADMaterial нужно разбить на One, Two и Three, где One – значение из преф., Two - значение из строки между «One +" "» и «/», Three - значение из строки после «/»; затем присвоить

oc9_MaterialSEPart1=<One>+"  "+<минусы>, где кол-во минусов определяется максимальным кол-вом символов в Two или Three плюс один
oc9_MaterialSEPart2=<пробелы>+<Two>+"%RT"+<пробелы>+<Three>, где кол-во пробелов равно кол-ву символов в One плюс три
 */

#ifndef NUM_ENTRIES
#define NUM_ENTRIES 1
#endif

char* parseMaterialSEPart1(char*);

int oc9_get_MaterialSEPart1_value(METHOD_message_t *mess, va_list args) {
	try {
		printf("%s\n", "Into oc9_get_MaterialSEPart1_value");
		va_list vargs;
		va_copy(vargs, args);
		ResultCheck erc;
		char **rvalue;
		char* object_name;
		char* prop;
		tag_t tProp = va_arg(vargs, tag_t);
		rvalue = va_arg(vargs, char**);
		va_end(vargs);

		tag_t object_t = mess->object_tag;
		int index = 1;
		int entryCount;
		int resultCount;
		char** entryNames;
		char** entryValues;
		tag_t query_t = NULL_TAG;

		printf("%s\n", "...1");

		erc = AOM_ask_value_string(object_t, "oc9_StockCompanyPart", &prop);
		if(strcmp("", prop)){
			*rvalue = (char*) MEM_alloc( strlen(" ") + 1);
			strcpy(*rvalue, " ");
			MEM_free(prop);
			return ITK_ok;
		}

		erc = AOM_ask_value_string(object_t, "oc9_StockCommercialPart", &prop);
		if(strcmp("", prop)){
			*rvalue = (char*) MEM_alloc( strlen(" ") + 1);
			strcpy(*rvalue, " ");
			MEM_free(prop);
			return ITK_ok;
		}

		erc = AOM_ask_value_string(object_t, "oc9_CADMaterial", &prop);
		if(strcmp("", prop)){
			printf("%s\n", "IS material");
			char entryNamesArray[NUM_ENTRIES][QRY_uid_name_size_c+1]	= { "object_name_material" };
			entryNames = (char**)MEM_alloc( NUM_ENTRIES * sizeof *entryNames );
			for( index=0; index<NUM_ENTRIES; index++ ){
				entryNames[index] = (char*)MEM_alloc( strlen(entryNamesArray[index] + 1) );
				if( entryNames[index] )
					strcpy( entryNames[index], entryNamesArray[index] );
			}

			entryValues = (char**) MEM_alloc(NUM_ENTRIES * sizeof(char *));
			entryValues[0] = (char *)MEM_alloc( strlen( prop ) + 1);
			strcpy(entryValues[0], prop );

			auto_itk_mem_free<tag_t> results_found;
			erc = QRY_find("IdealPLM_Mat1Material", &query_t);
			if(query_t==NULLTAG){
				printf("%s\n", "Query IdealPLM_Mat1Material not found!");
			} else {
				printf("%s\n", "Query IdealPLM_Mat1Material was found!");
				erc = QRY_execute(query_t, NUM_ENTRIES, entryNames, entryValues, &resultCount, results_found.operator tag_t **());
				if(resultCount>0){
					printf("%s\n", "result>0");
					char* result = parseMaterialSEPart1(prop);
					*rvalue = (char*) MEM_alloc( strlen(result) + 1);
					strcpy(*rvalue, result);
					MEM_free(result);

				} else {
					printf("%s\n", "result=0");
					*rvalue = (char*) MEM_alloc( strlen("Неверно задан материал") + 1);
					strcpy(*rvalue, "Неверно задан материал");
				}
			}
			MEM_free(entryNames[0]);
			MEM_free(entryNames);
			MEM_free(entryValues[0]);
			MEM_free(entryValues);
			MEM_free(prop);
			return ITK_ok;
		}
		*rvalue = (char*) MEM_alloc(strlen(" ") + 1);
		strcpy(*rvalue, " ");
	} catch (...) {
			return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}

char* parseMaterialSEPart1(char* input) {
	int i;
	char* result;

	size_t currentMaxLength = 0;
	int preferencePos = -1;
	int currentLength = 0;
	int firstSpacePos = 0;
	int similarity = 0;

	int n_values = 0;
	char **preference = NULL;
	PREF_initialize();
	PREF_ask_char_values("Oc9_SortamentTypes", &n_values, &preference);//TODO

	printf("%s\n", "-parsing");

	for (i = 0; i < n_values; i++) {
		int prefLength = strlen(preference[i]);
		int contains = ( !strncmp(input, preference[i], prefLength) && input[strlen(preference[i])]==' ' );
		if (currentMaxLength > prefLength) {
			break;
		}
		if ((currentMaxLength < prefLength) && contains) {
			currentMaxLength = prefLength;
			preferencePos = i;
		}
	}

	int hasDelimiter = (strchr(input, '/')!=NULL)?1:0;
	if (preferencePos != -1 && hasDelimiter) {
		printf("%s\n", "--parsing");
		char* temp = (char*) MEM_alloc(strlen(input)+1);
		strcpy(temp, input);


		char* firstPart = preference[preferencePos];
		printf("f=%s\n", firstPart);
		char* secondPart = strtok((temp+strlen(preference[preferencePos])+1),"/");
		printf("s=%s\n", secondPart);
		char* thirdPart = (temp + strlen(preference[preferencePos]) + strlen(secondPart) + 2);
		printf("t=%s\n", thirdPart);

		int extraLength = (strlen(secondPart)>=strlen(thirdPart))?strlen(secondPart):strlen(thirdPart);
		result = (char*) MEM_alloc(strlen(firstPart) + extraLength + 3 + 1); // Added 3 for 2 spaces and one extra -
		strcpy(result, firstPart);

		strcat(result, "  ");
		for(i = 0; i < extraLength + 1; i++){
			strcat(result, "-");
		}
		printf("%s\n", result);

		MEM_free(temp);
	} else {
		printf("=%s\n", "---parsing");
		result = (char*) MEM_alloc(strlen(input)+1);
		strcpy(result, input);
	}
	MEM_free(preference);
	preference = NULL;
	printf("=%s\n", result);
	return result;
}
