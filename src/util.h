
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

/**** UTILITIES **************************************************************/


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
