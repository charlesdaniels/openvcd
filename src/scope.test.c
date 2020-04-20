/* Copyright 2020 Charles Daniels
 *
 * This file is part of OpenVCD and is released under a BSD 3-clause license.
 * See the LICENSE file in the project root for more information */

#define _GNU_SOURCE
#include "stdio.h"

#include "test_util.h"
#include "scope.h"

void test_reference(void) {
	openvcd_reference* r;

	/* alloc and then dealloc should not leak */
	r = openvcd_alloc_reference("testident", 0, 5);
	should_not_be_null(r);
	should_equal(r->lsb_index, 0);
	should_equal(r->msb_index, 5);
	str_should_equal(r->identifier, "testident");
	openvcd_free_reference(r);

}

void test_var(void) {
	openvcd_var* v;
	openvcd_reference* r;
	openvcd_scope* s;

	/* basic test with parent-less var */
	r = openvcd_alloc_reference("testident", 0, 5);
	should_not_be_null(r);
	should_equal(r->lsb_index, 0);
	should_equal(r->msb_index, 5);
	str_should_equal(r->identifier, "testident");
	v = openvcd_alloc_var(NULL, OPENVCD_VAR_WIRE, 8, r, "abc");
	should_not_be_null(v);
	should_be_null(v->parent);
	should_equal(v->type, OPENVCD_VAR_WIRE);
	should_equal(v->width, 8);
	str_should_equal(v->identifier_code, "abc");
	should_equal(r, v->reference);
	openvcd_free_var(v); /* should also free r */

	/* more advanced test with parent also */
	s = openvcd_alloc_scope(NULL, "scope0", OPENVCD_SCOPE_BEGIN);
	should_not_be_null(s);
	should_be_null(s->parent);
	str_should_equal(s->identifier, "scope0");
	should_equal(s->type, OPENVCD_SCOPE_BEGIN);
	r = openvcd_alloc_reference("testident", 0, 5);
	should_not_be_null(r);
	should_equal(r->lsb_index, 0);
	should_equal(r->msb_index, 5);
	str_should_equal(r->identifier, "testident");
	v = openvcd_alloc_var(s, OPENVCD_VAR_WIRE, 8, r, "abc");
	should_not_be_null(v);
	should_not_be_null(v->parent);
	should_equal(v->type, OPENVCD_VAR_WIRE);
	should_equal(v->width, 8);
	str_should_equal(v->identifier_code, "abc");
	should_equal(r, v->reference);
	should_equal(v->parent, s);
	should_be_true(kh_containsk(openvcd_mvar, s->child_variables, "abc"));
	should_be_false(kh_containsk(openvcd_mvar, s->child_variables, "xyz"));
	openvcd_free_var(v);
	/* make sure that we remove ourselves from the parent correctly */
	should_be_false(kh_containsk(openvcd_mvar, s->child_variables, "abc"));
	openvcd_free_scope(s);

}

void test_scope() {
	/* TODO: should actually tests scopes...
	 *
	 * * nested scopes
	 * * root scopes
	 * * scopes with child scopes + child vars
	 */
}


int main(void) {
	test_reference();
	test_var();
	test_scope();
	return 0;
}
