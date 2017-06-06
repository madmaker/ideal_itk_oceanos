#include <grm.h>
#include <epm.h>
#include <item.h>
//#include <epm_toolkit_tc_utils.h>
#include <base_utils/ResultCheck.hxx>
#include "../process_error.hxx"
#include "../misc.hxx"
#include "oc9_add_to_targets_rev.hxx"

int oc9_add_to_targets_rev(EPM_action_message_t msg) {
	try {
		TC_write_syslog("-Into- oc9_add_to_targets_rev\n");
		tag_t
			*attachments,
			root_task,
			item_to_add = NULLTAG,
			item_rev_to_add = NULLTAG,
			*attachments_to_add;
		int
			*attachments_types,
			attachments_count = 0,
			*attachments_types_to_add,
			index = 0;

		ResultCheck erc;

		erc = EPM_ask_root_task(msg.task, &root_task);
		erc = EPM_ask_all_attachments(root_task, &attachments_count, &attachments, &attachments_types);

		if(attachments_count>0){
			attachments_types_to_add = (int*) MEM_alloc(sizeof(int)*attachments_count);
			attachments_to_add = (tag_t*) MEM_alloc(sizeof(tag_t)*attachments_count);
		}

		for(int i = 0; i < attachments_count; i++){
			if(attachments_types[i]==EPM_target_attachment){
				erc = ITEM_ask_latest_rev (attachments[i], &item_rev_to_add);
				attachments_to_add[index] = item_rev_to_add;
				attachments_types_to_add[index] = EPM_target_attachment;
				index++;
			}
		}

		if(index>0) {
			erc = EPM_add_attachments(root_task, index, attachments_to_add, attachments_types_to_add);
			MEM_free(attachments_to_add);
			MEM_free(attachments_types_to_add);
		}
		if(attachments_count>0) {
			MEM_free(attachments);
			MEM_free(attachments_types);
		}

	} catch (...){
			return sisw::process_error(true, true, false);
	}
	return ITK_ok;
}
