/* Copyright 2020 Charles Daniels
 *
 * This file is part of OpenVCD and is released under a BSD 3-clause license.
 * See the LICENSE file in the project root for more information */

#include "scope.h"

openvcd_scope* openvcd_alloc_scope(openvcd_scope* parent, char* identifier, openvcd_scope_type type) {
	openvcd_scope* s;
	int khret;
	khint_t k;

	s = malloc(sizeof(openvcd_scope));
	if (s == NULL) { return NULL; }


	s->identifier = strdup(identifier);
	if (s->identifier == NULL) {
		free(s);
		return NULL;
	}

	s->type = type;

	s->child_scopes = kh_init(openvcd_mscope);
	if (s->child_scopes == NULL) {
		free(s->identifier);
		free(s);
		return NULL;
	}

	s->child_variables = kh_init(openvcd_mvar);
	if (s->child_variables == NULL) {
		kh_destroy(openvcd_mscope, s->child_scopes);
		free(s->identifier);
		free(s);
		return NULL;
	}

	s->parent = parent;

	/* install ourselves into the parent */
	if (s->parent != NULL) {
		k = kh_put(openvcd_mscope, s->parent->child_scopes, s->identifier, &khret);
		if (!khret) {
			kh_destroy(openvcd_mvar, s->child_variables);
			kh_destroy(openvcd_mscope, s->child_scopes);
			free(s->identifier);
			free(s);
			return NULL;
		}
		kh_val(s->parent->child_scopes, k) = s;
	}

	return s;

}

void openvcd_free_scope(openvcd_scope* s) {
	const char* key;
	openvcd_scope* cs;
	openvcd_var* cv;

	OPENVCD_UNUSED(key);

	/* remove ourselves from the parent scope */
	if (s->parent != NULL) {
		kh_delk(openvcd_mscope, s->parent->child_scopes, s->identifier);
	}

	/* First we must free all child scopes recursively. */
	kh_foreach(s->child_scopes, key, cs,
		openvcd_free_scope(cs);
	);
	kh_destroy(openvcd_mscope, s->child_scopes);

	/* And all of our variables. */
	kh_foreach(s->child_variables, key, cv,
		openvcd_free_var(cv);
	);
	kh_destroy(openvcd_mvar, s->child_variables);

	free(s->identifier);

	free(s);
}

openvcd_reference* openvcd_alloc_reference(char* identifier, int lsb_index, int msb_index) {
	openvcd_reference* r;

	r = malloc(sizeof(openvcd_reference));
	if (r == NULL) {
		return NULL;
	}

	r->identifier = strdup(identifier);
	if (r->identifier == NULL) {
		free(r);
		return NULL;
	}

	r->msb_index = msb_index;
	r->lsb_index = lsb_index;

	return r;
}

void openvcd_free_reference(openvcd_reference* r) {
	free(r->identifier);
	free(r);
}



openvcd_var* openvcd_alloc_var(openvcd_scope* parent, openvcd_var_type type, unsigned int width, openvcd_reference* reference, char* identifier_code) {
	openvcd_var* v;
	int khret;
	khint_t k;

	v = malloc(sizeof(openvcd_var));
	if (v == NULL) {
		return NULL;
	}

	v->parent = parent;
	v->type = type;
	v->width = width;
	v->reference = reference;
	v->identifier_code = strdup(identifier_code);
	if (v->identifier_code == NULL) {
		free(v);
		return NULL;
	}

	/* install ourselves into the parent scope */
	if (v->parent != NULL) {
		k = kh_put(openvcd_mvar, v->parent->child_variables, v->identifier_code, &khret);
		/* TODO: should verify this means v->parent->child_variables
		 * won't end up with a dangling reference and leak memory.
		 *
		 * XXX: maybe should write a kh_putkv() for util.h?
		 */
		if (!khret) {
			free(v->identifier_code);
			free(v);
			return NULL;
		}
		kh_val(v->parent->child_variables, k) = v;
	}

	return v;
}

void openvcd_free_var(openvcd_var* v) {
	/* remove the parent's reference to us */
	if (v->parent != NULL) {
		kh_delk(openvcd_mvar, v->parent->child_variables, v->identifier_code);
	}

	if (v->reference != NULL) {
		openvcd_free_reference(v->reference);
	}

	free(v->identifier_code);

	free(v);
}
