#include <stdarg.h>
#include <stdlib.h>
#include <ug_va_copy.h>
#include <itk/bmf.h>
#include <epm/epm.h>
#include <epm/epm_task_template_itk.h>

int getArgByName(const BMF_extension_arguments_t* const p, const int arg_cnt,
		const char* paramName) {
	int i = 0;

	for (; i < arg_cnt; i++) {
		if (tc_strcmp(paramName, p[i].paramName) == 0)
			return i;
	}

	return -1;
}

int G4_bmf_extension_workflow_sample(METHOD_message_t *msg, va_list args) {
	int ifail = ITK_ok;

	tag_t* new_item = NULL;
	tag_t* new_rev = NULL;

	tag_t item_tag = NULLTAG;
	tag_t rev_tag = NULLTAG;
	tag_t job_tag = NULLTAG;
	tag_t template_tag = NULLTAG;

	char* item_id = NULL;
	char* item_name = NULL;
	char* type_name = NULL;
	char* rev_id = NULL;

	char relproc[BMF_EXTENSION_STRGVAL_size_c + 1] = { '\0' };
	char job_desc[WSO_desc_size_c + 1] = { '\0' };
	char job_name[WSO_name_size_c + 1] = { '\0' };

	int index = 0;
	int paramCount = 0;
	int companyid = 0;
	int attachment_type = EPM_target_attachment;

	BMF_extension_arguments_t* input_args = NULL;

	/********************/
	/*  Initialization  */
	/********************/
	//Get the parameters from the ITEM_create_msg
	va_list largs;
	va_copy(largs, args);
	item_id = va_arg( largs, char *);
	item_name = va_arg(largs, char *);
	type_name = va_arg(largs, char *);
	rev_id = va_arg(largs, char *);
	new_item = va_arg(largs, tag_t *);
	new_rev = va_arg(largs, tag_t *);
	item_tag = *new_item;
	rev_tag = *new_rev;
	va_end(largs);

	// Extract the user arguments from the message
	ifail = BMF_get_user_params(msg, &paramCount, &input_args);

	if (ifail == ITK_ok && paramCount >= 2) {
		index = getArgByName(input_args, paramCount, "Release Process");

		if (index != -1)
			tc_strcpy(relproc, input_args[index].arg_val.str_value);

		index = getArgByName(input_args, paramCount, "Company ID");

		if (index != -1)
			companyid = input_args[index].arg_val.int_value;

		MEM_free(input_args);

		if (relproc != NULL && rev_tag != NULLTAG) {
			sprintf(job_name, "%s/%s-%d Job", item_id, rev_id, companyid);
			sprintf(job_desc,
					"Auto initiate job for Item/ItemRevision (%s/%s-%d)",
					item_id, rev_id, companyid);

			//Get the template tag.
			ifail = EPM_find_template(relproc, 0, &template_tag);

			if (ifail != ITK_ok)
				return ifail;

			// Create the job, initiate it and attach it to the item revision.
			ifail = EPM_create_process(job_name, job_desc, template_tag, 1,
					&rev_tag, &attachment_type, &job_tag);

			if (ifail != ITK_ok)
				return ifail;
		} else {
			// User supplied error processing.
		}
	}

	return ifail;
}
