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
typedef uint16_t rh_idx_t;


/**
 * A hash table is a collection of hash table entries.
 */
typedef struct {
	PsmAddress	value;
	char		key[32];
	rh_idx_t	delta;
} rh_elt_t;


/**
 * Meta-data for the hash table and a pointer to its first entry.
 */
typedef struct {
	rh_elt_t	buckets[ARMUR_VIMAGE_HASH_ENTRIES];
	rh_idx_t	num_bkts;
	rh_idx_t	num_elts;
	rh_idx_t	max_delta;
} rhht_t;


/*
 * +--------------------------------------------------------------------------+
 * |						  FUNCTION PROTOTYPES  							  +
 * +--------------------------------------------------------------------------+
 */


extern PsmAddress	rhht_create();
extern PsmAddress	rhht_retrieve_key(PsmAddress rhhtAddr, char *key);
extern int		rhht_insert(PsmAddress rhhtAddr, char *key, PsmAddress value);
extern void		rhht_release(PsmAddress rhhtAddr);
extern void		rhht_del_key(PsmAddress rhhtAddr, void *key);

#endif /* _ARMUR_RHHT_H_ */
