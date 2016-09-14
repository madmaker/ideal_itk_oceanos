#ifndef OC9_ADD_TO_TARGETS_REV_HXX_
#define OC9_ADD_TO_TARGETS_REV_HXX_

#include <tc/tc.h>
#include <epm.h>

/**
 * Handler который для всех item целей ищет последнюю ревизию и добавляет ее в цели
 */

int oc9_add_to_targets_rev(EPM_action_message_t);

#endif
