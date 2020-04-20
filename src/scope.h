/* Copyright 2020 Charles Daniels
 *
 * This file is part of OpenVCD and is released under a BSD 3-clause license.
 * See the LICENSE file in the project root for more information */

/**** OVERVIEW ***************************************************************/

/* This file defines OpenVCD's model of a VCD scope, which is built on top of
 * the excellent khash.h hash table library.
 */

#ifndef OPENVCD_SCOPE_H
#define OPENVCD_SCOPE_H

/**** INCLUDES ***************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "khash.h"
#include "vec.h"
#include "util.h"

/**** TYPES ******************************************************************/

typedef vec_t(struct openvcd_scope*) openvcd_scopelist;

typedef vec_t(struct openvcd_var*) openvcd_varlist;

/* mapping of strings to OpenVCD variables */
KHASH_MAP_INIT_STR(openvcd_mvar, struct openvcd_var_t*)

/* mapping of strings to OpenVCD scopes */
KHASH_MAP_INIT_STR(openvcd_mscope, struct openvcd_scope_t*)

/* mapping of strings to lists of OpenVCD variables */
KHASH_MAP_INIT_STR(openvcd_mvarlist, openvcd_varlist*)

typedef enum {
	OPENVCD_SCOPE_BEGIN,
	OPENVCD_SCOPE_FORK,
	OPENVCD_SCOPE_FUNCTION,
	OPENVCD_SCOPE_MODULE,
	OPENVCD_SCOPE_TASK,
} openvcd_scope_type;

typedef struct openvcd_scope_t {

	/* Parent scope, NULL if this is the root scope. */
	struct openvcd_scope_t* parent;

	char* identifier;
	openvcd_scope_type type;

	/* Note that OpenVCD is not capable of representing child scopes or
	 * variables with duplicated names... the spec does not seem to
	 * explicitly forbid this, but it seems unlikely any correctly working
	 * tool would generate such a VCD file */
	khash_t(openvcd_mscope)* child_scopes;
	khash_t(openvcd_mvar)* child_variables;

} openvcd_scope;

typedef struct openvcd_reference_t {
	char* identifier;

	/* To encode a single-bit selection, set msb_index and lsb_index to
	 * be the same. */
	int msb_index;
	int lsb_index;
} openvcd_reference;

typedef enum {
	OPENVCD_VAR_EVENT,
	OPENVCD_VAR_INTEGER,
	OPENVCD_VAR_PARAMETER,
	OPENVCD_VAR_REAL,
	OPENVCD_VAR_REALTIME,
	OPENVCD_VAR_REG,
	OPENVCD_VAR_SUPPLY0,
	OPENVCD_VAR_SUPPLY1,
	OPENVCD_VAR_TIME,
	OPENVCD_VAR_TRI,
	OPENVCD_VAR_TRIAND,
	OPENVCD_VAR_TRIOR,
	OPENVCD_VAR_TRIREG,
	OPENCVD_VAR_TRI0,
	OPENVCD_VAR_TRI1,
	OPENVCD_VAR_WANT,
	OPENVCD_VAR_WIRE,
	OPENVCD_VAR_WOR
} openvcd_var_type;

typedef struct openvcd_var_t {
	openvcd_scope* parent;
	openvcd_var_type type;
	unsigned int width;
	char* identifier_code;
	openvcd_reference* reference;
} openvcd_var;

/**** PROTOTYPES *************************************************************/

/**
 * @brief Allocate a new scope object.
 *
 * Note that the identifier string will be strdup()-ed, and should be free-ed
 * by the caller after calling openvcd_alloc_scope();
 *
 * @param parent The parent scope, may be NULL if none.
 * @param identifier The scope identifier.
 * @param type The scope type.
 *
 * @return A new scope, allocated via malloc(). The caller should later free
 * it using openvcd_free_scope(). Returns NULL on error.
 */
openvcd_scope* openvcd_alloc_scope(openvcd_scope* parent, char* identifier, openvcd_scope_type type);

/**
 * @brief Free a previously allocated scope.
 *
 * If a scope is free-ed all of the child objects including child_scopes,
 * child_variables will also be free-ed using the appropriate functions.
 *
 * This function can be used to destroy a scope at runtime.
 *
 * This function will also walk back up into the parent scope and remove the
 * references to this scope from parent->child_scopes, if the parent is
 * non-NULL.
 *
 * @param s
 */
void openvcd_free_scope(openvcd_scope* s);

/**
 * @brief Allocate a new OpenVCD reference.
 *
 * Note that the identifier is internally strdup()-ed, so it should be free-ed
 * by the caller.
 *
 * @param identifier
 * @param msb_index
 * @param lsb_index
 *
 * @return Newly allocated reference object, must later be free-ed using
 * openvcd_free_reference();
 */
openvcd_reference* openvcd_alloc_reference(char* identifier, int lsb_index, int msb_index);

/**
 * @brief Free a previously allocated OpenVCD reference.
 *
 * @param r
 */
void openvcd_free_reference(openvcd_reference* r);

/**
 * @brief Allocates a new OpenVCD variable.
 *
 * Note that by passing a reference to this function, it's lifecycle is now
 * managed by this var object, see openvcd_alloc_var().
 *
 * @param parent
 * @param type
 * @param width
 * @param reference
 * @param identifier_code Will be strdup()-ed, should be freed by caller.
 *
 * @return The new variable, must later be free-ed using openvcd_free_var().
 */
openvcd_var* openvcd_alloc_var(openvcd_scope* parent, openvcd_var_type type, unsigned int width, openvcd_reference* reference, char* identifier_code);

/**
 * @brief Free a previously allocated OpenVCD variable.
 *
 * If the reference is non-NULL, this will also call openvcd_free_reference()
 * thereon.
 *
 * This function will also walk up into the parent object and remove the
 * reference in parent->child_ariables from the parent, if the parent
 * is non-NULL.
 *
 * @param v
 */
void openvcd_free_var(openvcd_var* v);


#endif /* OPENVCD_SCOPE_H */
