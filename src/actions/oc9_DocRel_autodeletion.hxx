//@<COPYRIGHT>@
//==================================================
//Copyright $2015.
//Siemens Product Lifecycle Management Software Inc.
//All Rights Reserved.
//==================================================
//@<COPYRIGHT>@

/* 
 * @file 
 *
 *   This file contains the declaration for the Extension oc9_DocRelAutoDeletion
 *
 */
 
#ifndef OC9_DOCREL_AUTODELETION_HXX
#define OC9_DOCREL_AUTODELETION_HXX
#include <tccore/method.h>
#include <aom.h>
#include <item.h>
#include <tccore/aom_prop.h>
#include <tccore/grm.h>
#include <tccore/tctype.h>
#include <iostream>
#ifdef __cplusplus
         extern "C"{
#endif
                 
int oc9_DocRel_autodeletion(METHOD_message_t* msg, va_list args);
                 
#ifdef __cplusplus
                   }
#endif

#endif
