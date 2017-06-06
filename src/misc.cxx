#include "misc.hxx"
/**
 *
 */
//cppcheck-suppress unusedFunction
bool starts_with(const std::string & param, const std::string & what) {
	if (param.size() < what.size())
		return false;
	return !param.compare(0, what.size(), what);
}

void appendToResult(char* base_string, char* prefix, char* to_append, int max_result_length){
	if(to_append!=NULL && strlen(to_append)>0){
		if((strlen(prefix)+strlen(to_append)) < (max_result_length - strlen(base_string))){
			strcat(base_string, prefix);
			strcat(base_string, to_append);
		}
	}
}

char* getId(char* input){
	TC_write_syslog("-into getID\n");
	TC_write_syslog(input);
	char* result = NULL;
	if(input!=NULL){
		size_t length = strcspn(input, " -");
		if(length>0){
			TC_write_syslog("length>0\n");
			result = (char*) MEM_alloc(length * sizeof(char) + 1);
			strncpy(result, input, length);
			result[length+1] = '\0';
			TC_write_syslog("result=%s\n", result);
		}
	}
	TC_write_syslog("-exiting getID\n");
	return result;
}

int cmpfunc(const void * a, const void * b) {
	return (*(int*) a - *(int*) b);
}

bool isvalueinarray(int val, int *arr, int size) {
	int i;
	for (i = 0; i < size; i++) {
		if (arr[i] == val)
			return true;
	}
	return false;
}
