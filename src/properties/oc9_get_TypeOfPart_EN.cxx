#include <stdarg.h>
#include <tc/tc.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <ug_va_copy.h>

#include <base_utils/ResultCheck.hxx>
#include "../misc.hxx"
#include "../process_error.hxx"
#include "oc9_get_TypeOfPart_EN.hxx"

int oc9_get_typeOfPart_en(METHOD_message_t *mess, va_list args){
	try{
		printf("%s\n", "Into oc9_get_typeOfPart_en");
		va_list vargs;
		va_copy(vargs, args);
		char **rvalue = va_arg(vargs, char**);
		va_end(vargs);

		tag_t object_t = mess->object_tag;
		char* typeOfPart = NULL;
		char* result;
		ResultCheck erc;

		erc = AOM_ask_value_string(object_t, "oc9_TypeOfPart", &typeOfPart);
		if(typeOfPart!=NULL){
			if(strcmp(typeOfPart, "Стандартное изделие") == 0){
				*rvalue = (char*) MEM_alloc(sizeof(char)*strlen("Standart") + 1);
				strcpy(*rvalue, "Standart");
				MEM_free(typeOfPart);
			} else if(strcmp(typeOfPart, "Прочее изделие") == 0) {
				*rvalue = (char*) MEM_alloc(sizeof(char)*strlen("Other") + 1);
				strcpy(*rvalue, "Other");
				MEM_free(typeOfPart);
			} else if(strcmp(typeOfPart, "Деталь") == 0){
				*rvalue = (char*) MEM_alloc(sizeof(char)*strlen("Part") + 1);
				strcpy(*rvalue, "Part");
				MEM_free(typeOfPart);
			} else if(strcmp(typeOfPart, "Сборочная единица") == 0){
				*rvalue = (char*) MEM_alloc(sizeof(char)*strlen("Assembly") + 1);
				strcpy(*rvalue, "Assembly");
				MEM_free(typeOfPart);
			} else if(strcmp(typeOfPart, "Комплект") == 0){
				*rvalue = (char*) MEM_alloc(sizeof(char)*strlen("Set") + 1);
				strcpy(*rvalue, "Set");
				MEM_free(typeOfPart);
			} else if(strcmp(typeOfPart, "Комплекс") == 0){
				*rvalue = (char*) MEM_alloc(sizeof(char)*strlen("Complex") + 1);
				strcpy(*rvalue, "Complex");
				MEM_free(typeOfPart);
			} else if(strcmp(typeOfPart, "Геометрия материала") == 0){
				*rvalue = (char*) MEM_alloc(sizeof(char)*strlen("GeomOfMat") + 1);
				strcpy(*rvalue, "GeomOfMat");
				MEM_free(typeOfPart);
			} else if(strcmp(typeOfPart, "Материал") == 0){
				*rvalue = (char*) MEM_alloc(sizeof(char)*strlen("Material") + 1);
				strcpy(*rvalue, "Material");
				MEM_free(typeOfPart);
			}
		}

	} catch (...) {
		return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}
