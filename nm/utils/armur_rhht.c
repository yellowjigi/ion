/******************************************************************************
 **                           COPYRIGHT NOTICE
 **      (c) 2018 The Johns Hopkins University Applied Physics Laboratory
 **                         All rights reserved.
 ******************************************************************************/

/*****************************************************************************
 **
 ** File Name: rhht.c
 **
 ** Modified from the original source: ION_ROOT_DIR/nm/shared/utils/rhht.c
 **
 *****************************************************************************/
#include "armur_rhht.h"

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
 * |					   Private Functions  								  +
 * +--------------------------------------------------------------------------+
 */


static int	_compare(char *key1, char *key2)
{
	return strcmp(key1, key2);
}


/******************************************************************************
 *
 * hash_key
 *
 *****************************************************************************/
static rh_idx_t	_hash(rhht_t *ht, char *key)
{
	/*	MurmurHash2
	 *	Original source:
	 *	github.com/aappleby/smhasher/blob/master/src/MurmurHash2.cpp	*/
	int		len = strlen(key);
	uint32_t	seed = 0;

	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.
	
	const uint32_t m = 0x5bd1e995;
	const int r = 24;
	
	// Initialize the hash to a 'random' value
	
	uint32_t h = seed ^ len;
	
	// Mix 4 bytes at a time into the hash
	
	const unsigned char *data = (const unsigned char *)key;
	
	while (len >= 4)
	{
		uint32_t k = *(uint32_t *)data;
		
		k *= m;
		k ^= k >> r;
		k *= m;
		
		h *= m;
		h ^= k;
		
		data += 4;
		len -= 4;
	}
	
	// Handle the last few bytes of the input array
	
	switch (len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
	};
	
	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	
	return h % ht->num_bkts;
}

static void	_delete(rh_elt_t *elt)
{
	PsmPartition	wm = getIonwm();

	psm_free(wm, elt->value);
}

static void p_rhht_bkwrd_shft(rhht_t *ht, rh_idx_t idx)
{
	rh_idx_t	i;
	rh_idx_t	next_idx;

	CHKVOID(ht);
	CHKVOID(idx < ht->num_bkts);

	if(ht->buckets[idx].value != 0)
	{
		return;
	}

	/* Better to avoid recursion when you have very large hash tables. */
	for(i = 0; i < ht->num_bkts; i++)
	{
		/* Calculate the next index. */
		next_idx = (idx + 1) % ht->num_bkts;

		/* If the next bucket is empty or perfect, we are done shifting. */
		if (ht->buckets[next_idx].value == 0
		|| ht->buckets[next_idx].delta == 0)
		{
			return;
		}

		/* pull the next index into the one that was just vacated. */
		ht->buckets[idx] = ht->buckets[next_idx];

		/* We just got 1 closer to our ideal index. */
		ht->buckets[idx].delta--;

		/* Set the next index to be empty. */
		memset(&(ht->buckets[next_idx]), 0, sizeof(rh_elt_t));

		/* adjust to consider the newly vacated slot and do it again... */
		idx = next_idx;
	}
}

static rh_idx_t p_rh_calc_ideal_idx(rh_idx_t size, rh_idx_t cur, rh_idx_t delta)
{
	return (cur >= delta) ? (cur - delta) : (size - delta - cur);
}


/******************************************************************************
 *
 * hash_insert_rh_elt_t
 *
 *****************************************************************************/
static rh_idx_t p_rh_calc_placement(rhht_t *ht, rh_idx_t idx, rh_elt_t *elt)
{
	rh_idx_t	iter = 0;
	rh_idx_t	index = idx;

	/*
	 * Walk the array until either:
	 *    1. Our current spot is empty.
	 *    2. Our delta is larger than the delta of the item in this spot.
	 *    3. Our delta is the same but we hash lower (tie breaker).
	 *    4. We have walked the list fully once.
	 */
	
	for (iter = 0; iter < ht->num_bkts; iter++)
	{
		if ((ht->buckets[index].value == 0) /* Condition 1 - Empty Spot. */
		|| (elt->delta > ht->buckets[index].delta) /* Condition 2 - We are poorer. */
		|| ((elt->delta == ht->buckets[index].delta) /* Condition 3 - We are even. */
			&& (idx < _hash(ht, ht->buckets[index].key))))
		{
			break;
		}
		else	/* Try the next spot. */
		{
			elt->delta++;
			index = (idx + elt->delta) % ht->num_bkts;
		}
	}

	return index;
}


/*
 * +--------------------------------------------------------------------------+
 * |					   Public Functions  								  +
 * +--------------------------------------------------------------------------+
 */


/******************************************************************************
 *
 * \par Function Name: rhht_create
 *
 *****************************************************************************/
PsmAddress	ARMUR_rhht_create()
{
	PsmPartition	wm = getIonwm();
	PsmAddress	rhhtAddr;
	rhht_t		*ht;

	if ((rhhtAddr = psm_zalloc(wm, sizeof(rhht_t))) == 0)
	{
		return 0;
	}

	ht = (rhht_t *)psp(wm, rhhtAddr);
	memset(ht, 0, sizeof(rhht_t));

        ht->num_bkts = ARMUR_VIMAGE_HASH_ENTRIES;

	return rhhtAddr;
}


/******************************************************************************
 *
 * \par Function Name: rhht_find
 *
 *****************************************************************************/
static int	rhht_find(rhht_t *ht, char *key, rh_idx_t *idx)
{
	rh_idx_t	i;
	rh_idx_t	tmp;

	CHKZERO(key);

	if (ht->num_elts == 0)
	{
		// HT is empty. Nothing to be found (and not an error)
		return RH_NOT_FOUND;
	}
    
	CHKZERO(idx);

	/* Step 1: Hash the item. */
	tmp = _hash(ht, key);

	/* Step 2: If nothing is there, it.. isn't there. */
	if (ht->buckets[tmp].value == 0)
	{
		return RH_NOT_FOUND;
	}

	for (i = 0; i < ht->num_bkts; i++)
	{
		if (_compare(key, ht->buckets[tmp].key) == 0)
		{
			if (idx != NULL)
			{
				*idx = tmp;
			}
			return RH_OK;
		}

		tmp = (tmp + 1) % ht->num_bkts;

		/* If we run into someone who is in their ideal slot, we
		 * can stop looking as we are at the end of our virtual bucket.
		 */
		if (ht->buckets[tmp].delta == 0)
		{
			break;
		}
	}

	tmp = ht->num_bkts;

	if (idx != NULL)
	{
		*idx = tmp;
	}

	return RH_NOT_FOUND;
}

PsmAddress	ARMUR_rhht_retrieve_key(PsmAddress rhhtAddr, char *key)
{
	PsmPartition	wm = getIonwm();
	rhht_t		*ht;
	rh_idx_t	idx;

	ht = (rhht_t *)psp(wm, rhhtAddr);
	if (rhht_find(ht, key, &idx) != RH_OK)
	{
		return 0;
	}

	return ht->buckets[idx].value;
}


/******************************************************************************
 *
 * \par Function Name: rhht_insert
 *
 *****************************************************************************/
int	ARMUR_rhht_insert(PsmAddress rhhtAddr, char *key, PsmAddress value)
{
	PsmPartition	wm = getIonwm();
	rhht_t		*ht;
	rh_idx_t	actual_idx = 0;
	rh_idx_t	ideal_idx;
	rh_idx_t	iter = 0;
	rh_elt_t	elt;

	ht = (rhht_t *)psp(wm, rhhtAddr);

	if (ht->num_elts == ht->num_bkts)
	{	
		return RH_FULL;
	}

	strcpy(elt.key, key);
	elt.value = value;
	elt.delta = 0;
	ideal_idx = _hash(ht, key);

	for (iter = 0; (iter < ht->num_bkts) && (elt.value != 0); iter++)
	{
		/* Get the place where the item should live. */
		actual_idx = p_rh_calc_placement(ht, ideal_idx, &elt);

		/*
		 * Store the new item at its new index. Remember what was there,
		 * if anything, because we may have to propagate...
		 */
		rh_elt_t temp = ht->buckets[actual_idx];
		ht->buckets[actual_idx] = elt;

		/*
		 * If there was an item at the actual_idx, we have bumped it.
		 * Save the bumped item as we will need to go back in and
		 * rehash it. We do this by:
		 * - saving its data pointer.
		 * - calculating it's ideal index.
		 * - setting its delta to 0. We will re-calculate its delta
		 *   as part of finding its new home.
		 */
		strcpy(elt.key, temp.key);
		elt.value = temp.value;
		elt.delta = 0;

		/*
		 * We don't have to re-hash the bumped item. We can calculate its ideal
		 * index as a function of where it had been living (actual_idx) and
		 * what it's delta was (temp.delta).
		 */
		ideal_idx = p_rh_calc_ideal_idx(ht->num_bkts, actual_idx, temp.delta);
	}

	ht->num_elts++;

	return RH_OK;
}


/******************************************************************************
 *
 * delete_rhht_t
 *
 ******************************************************************************/
static void	rhht_del_idx(rhht_t *ht, rh_idx_t idx)
{
	CHKVOID(ht);
	CHKVOID(idx < ht->num_bkts);

	if ((ht->buckets[idx].value != 0))
	{
		_delete(&(ht->buckets[idx]));

	}
	memset(&(ht->buckets[idx]), 0, sizeof(rh_elt_t));
	ht->num_elts--;

	p_rhht_bkwrd_shft(ht, idx);
}

void	ARMUR_rhht_del_key(PsmAddress rhhtAddr, void *item)
{
	PsmPartition	wm = getIonwm();
	rhht_t		*ht;
	rh_idx_t	idx;

	ht = (rhht_t *)psp(wm, rhhtAddr);
	if (rhht_find(ht, item, &idx) == RH_OK)
	{
		rhht_del_idx(ht, idx);
	}
}

void	ARMUR_rhht_release(PsmAddress rhhtAddr)
{
	PsmPartition	wm = getIonwm();
	rhht_t		*ht;
	rh_idx_t	i;

	CHKVOID(rhhtAddr);

	ht = (rhht_t *)psp(wm, rhhtAddr);
	for (i = 0; i < ht->num_bkts; i++)
	{
		if (ht->buckets[i].value != 0)
		{
			_delete(&(ht->buckets[i]));
		}
	}
	psm_free(wm, rhhtAddr);
}
