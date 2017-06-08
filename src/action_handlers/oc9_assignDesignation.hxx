#ifndef OC9_ASSIGNDESIGNATION_HXX_
#define OC9_ASSIGNDESIGNATION_HXX_

#include <tc/tc.h>
#include <epm.h>

char* getFreeNum(char**, int);
char* replaceItemId(char*, char*);
char* getExtension(char*);
bool isRenamableDatasetType(char*);
void renameDatasetAndNamedRefs(tag_t, tag_t);
int oc9_assignDesignation(EPM_action_message_t);

#endif
