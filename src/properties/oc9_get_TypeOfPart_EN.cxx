#include <stdarg.h>
#include <tc/tc.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <ug_va_copy.h>

#include <base_utils/ResultCheck.hxx>
#include "../misc.hxx"
#include "../process_error.hxx"
#include "oc9_get_TypeOfPart_EN.hxx"

int oc9_get_TypeOfPart_EN(METHOD_message_t *mess, va_list args){
	try{
		tag_t prop_tag;
		char **rvalue;
		va_list vargs;
		va_copy(vargs, args);
		prop_tag = va_arg(vargs, tag_t);
		rvalue = va_arg(vargs, char**);
		va_end(vargs);

		tag_t object_t = mess->object_tag;
		char* typeOfPart = NULL;
		char* result;
		ResultCheck erc;

		erc = AOM_ask_value_string(object_t, "oc9_TypeOfPart", &typeOfPart);
		if(typeOfPart!=NULL){
			if(strcmp(typeOfPart, "Стандартное изделие") == 0){
				*rvalue = (char*) MEM_alloc(strlen("Standart") + 1);
				strcpy(*rvalue, "Standart");
			} else if(strcmp(typeOfPart, "Прочее изделие") == 0) {
				*rvalue = (char*) MEM_alloc(strlen("Other") + 1);
				strcpy(*rvalue, "Other");
			} else if(strcmp(typeOfPart, "Деталь") == 0){
				*rvalue = (char*) MEM_alloc(strlen("Part") + 1);
				strcpy(*rvalue, "Part");
			} else if(strcmp(typeOfPart, "Сборочная единица") == 0){
				*rvalue = (char*) MEM_alloc(strlen("Assembly") + 1);
				strcpy(*rvalue, "Assembly");
			} else if(strcmp(typeOfPart, "Комплект") == 0){
				*rvalue = (char*) MEM_alloc(strlen("Set") + 1);
				strcpy(*rvalue, "Set");
			} else if(strcmp(typeOfPart, "Комплекс") == 0){
				*rvalue = (char*) MEM_alloc(strlen("Complex") + 1);
				strcpy(*rvalue, "Complex");
			} else if(strcmp(typeOfPart, "Геометрия материала") == 0){
				*rvalue = (char*) MEM_alloc(strlen("GeomOfMat") + 1);
				strcpy(*rvalue, "GeomOfMat");
			} else if(strcmp(typeOfPart, "Материал") == 0){
				*rvalue = (char*) MEM_alloc(strlen("Material") + 1);
				strcpy(*rvalue, "Material");
			}
			MEM_free(typeOfPart);
		}
		return ITK_ok;

	} catch (...) {
		return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}
