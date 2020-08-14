/******************************************************************************
 **                           COPYRIGHT NOTICE
 **      (c) 2018 The Johns Hopkins University Applied Physics Laboratory
 **                         All rights reserved.
 ******************************************************************************/

/*****************************************************************************
 **
 ** File Name: rhht.h
 **
 ** Modified from the original source: ION_ROOT_DIR/nm/shared/utils/rhht.h
 **
 *****************************************************************************/

#ifndef _ARMUR_RHHT_H_
#define _ARMUR_RHHT_H_

#include "stdint.h"
#include "ion.h"


/*
 * +--------------------------------------------------------------------------+
 * |							  CONSTANTS  								  +
 * +--------------------------------------------------------------------------+
 */
#define RH_ERROR      0
#define RH_OK         1
#define RH_SYSERR    -1
#define RH_DUPLICATE -2
#define RH_NOT_FOUND -3
#define RH_FULL      -4

/*	ARMUR Hash values	*/
#define ARMUR_VIMAGE_HASH_ENTRIES	31


/*
 * +--------------------------------------------------------------------------+
 * |							  	MACROS  								  +
 * +--------------------------------------------------------------------------+
 */


/*
 * +--------------------------------------------------------------------------+
 * |							  DATA TYPES  								  +
 * +--------------------------------------------------------------------------+
 */


/*
 * +--------------------------------------------------------------------------+
 * |						  FUNCTION PROTOTYPES  							  +
 * +--------------------------------------------------------------------------+
 */


extern PsmAddress	ARMUR_rhht_create();
extern PsmAddress	ARMUR_rhht_retrieve_key(PsmAddress rhhtAddr, char *key);
extern int		ARMUR_rhht_insert(PsmAddress rhhtAddr, char *key, PsmAddress value);
extern void		ARMUR_rhht_del_key(PsmAddress rhhtAddr, void *key);
extern void		ARMUR_rhht_release(PsmAddress rhhtAddr);

#endif /* _ARMUR_RHHT_H_ */
