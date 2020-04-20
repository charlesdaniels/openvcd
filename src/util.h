
/* Copyright 2020 Charles Daniels
 *
 * This file is part of OpenVCD and is released under a BSD 3-clause license.
 * See the LICENSE file in the project root for more information */

/**** OVERVIEW ***************************************************************/

/* This file contains utility methods and macros for OpenVCD */

#ifndef OPENVCD_UTIL_H
#define OPENVCD_UTIL_H

/**** INCLUDES ***************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "khash.h"

/**** UTILITIES **************************************************************/

#define OPENVCD_UNUSED(x) (void)(x)

/**
 * @brief Works identically to kh_del(name, h, k), but key is a key rather than
 * a khint_t.
 *
 * @param _name
 * @param _h
 * @param _key
 *
 * @return
 */
#define kh_delk(_name, _h, _key) do { \
		khint_t _kh; \
		_kh = kh_get(_name, _h, _key); \
		if (_kh != kh_end(_h)) { \
			kh_del(_name, _h, _kh); \
		} \
	} while(0)

/**
 * @brief Returns true if the specified khash contains the given key.
 *
 * @param _name
 * @param _h
 * @param _k the key (not a khint_t!)
 */
#define kh_containsk(_name, _h, _k) \
	((kh_get(_name, _h, _k) != kh_end(_h)) ? \
		(1 == kh_exist(_h, kh_get(_name, _h, _k))) : false)


/**** PROTOTYPES *************************************************************/

/**
 * @brief Returns a version of s such that it contains no characters not in
 * filter.
 *
 * The return will be a newly malloc-ed string which should be free-ed by the
 * caller.
 *
 * @param s
 * @param filter
 *
 * @return
 */
char* openvcd_charfilter(char* s, char* filter);

#endif /* OPENVCD_UTIL_H */
