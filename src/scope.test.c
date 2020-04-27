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
	khint_t k;

	/* allocate everything and make sure the constructors work as they
	 * should */
	openvcd_scope* root = \
		openvcd_alloc_scope(NULL, "root", OPENVCD_SCOPE_MODULE);
	should_not_be_null(root);
	str_should_equal(root->identifier, "root");
	should_equal(root->type, OPENVCD_SCOPE_MODULE);
	should_equal(root->parent, NULL);
	openvcd_scope* child1 = \
		openvcd_alloc_scope(root, "child1", OPENVCD_SCOPE_MODULE);
	should_not_be_null(child1);
	str_should_equal(child1->identifier, "child1");
	should_equal(child1->parent, root);
	should_equal(child1->type, OPENVCD_SCOPE_MODULE);
	openvcd_scope* child2 = \
		openvcd_alloc_scope(root, "child2", OPENVCD_SCOPE_MODULE);
	should_not_be_null(child2);
	str_should_equal(child2->identifier, "child2");
	should_equal(child2->parent, root);
	should_equal(child2->type, OPENVCD_SCOPE_MODULE);

	openvcd_reference* root_var1_ref = \
		openvcd_alloc_reference("var1", 0, 10);
	should_not_be_null(root_var1_ref);
	str_should_equal(root_var1_ref->identifier, "var1");
	openvcd_reference* root_var2_ref = \
		openvcd_alloc_reference("var2", 5, 10);
	should_not_be_null(root_var2_ref);
	str_should_equal(root_var2_ref->identifier, "var2");
	openvcd_reference* child1_var3_ref = \
		openvcd_alloc_reference("var3", 0, 7);
	should_not_be_null(child1_var3_ref);
	str_should_equal(child1_var3_ref->identifier, "var3");
	openvcd_reference* child2_var4_ref = \
		openvcd_alloc_reference("var4", 0, 7);
	should_not_be_null(child2_var4_ref);
	str_should_equal(child2_var4_ref->identifier, "var4");

	openvcd_var* root_var1 = \
		openvcd_alloc_var(root, OPENVCD_VAR_WIRE, 11, root_var1_ref, "001");
	should_not_be_null(root_var1);
	should_equal(root_var1->parent, root);
	should_equal(root_var1->type, OPENVCD_VAR_WIRE);
	should_equal(root_var1->width, 11);
	should_not_be_null(root_var1->reference);
	should_equal(root_var1->reference, root_var1_ref);
	str_should_equal(root_var1->identifier_code, "001");
	openvcd_var* root_var2 = \
		openvcd_alloc_var(root, OPENVCD_VAR_WIRE, 11, root_var2_ref, "002");
	should_not_be_null(root_var2);
	should_equal(root_var2->parent, root);
	should_equal(root_var2->type, OPENVCD_VAR_WIRE);
	should_equal(root_var2->width, 11);
	should_not_be_null(root_var2->reference);
	should_equal(root_var2->reference, root_var2_ref);
	str_should_equal(root_var2->identifier_code, "002");
	openvcd_var* child1_var3 = \
		openvcd_alloc_var(child1, OPENVCD_VAR_WIRE, 8, child1_var3_ref, "003");
	should_not_be_null(child1_var3);
	should_equal(child1_var3->parent, child1);
	should_equal(child1_var3->type, OPENVCD_VAR_WIRE);
	should_equal(child1_var3->width, 8);
	should_not_be_null(child1_var3->reference);
	should_equal(child1_var3->reference, child1_var3_ref);
	str_should_equal(child1_var3->identifier_code, "003");
	openvcd_var* child2_var4 = \
		openvcd_alloc_var(child2, OPENVCD_VAR_WIRE, 8, child2_var4_ref, "004");
	should_not_be_null(child2_var4);
	should_equal(child2_var4->parent, child2);
	should_equal(child2_var4->type, OPENVCD_VAR_WIRE);
	should_equal(child2_var4->width, 8);
	should_not_be_null(child2_var4->reference);
	should_equal(child2_var4->reference, child2_var4_ref);
	str_should_equal(child2_var4->identifier_code, "004");

	/* test that all the hash tables are set up properly */
	should_be_true(kh_containsk(openvcd_mscope, root->child_scopes, "child1"));
	should_be_true(kh_containsk(openvcd_mscope, root->child_scopes, "child2"));
	should_be_true(kh_containsk(openvcd_mvar, root->child_variables, "001"));
	should_be_true(kh_containsk(openvcd_mvar, root->child_variables, "002"));
	should_be_true(kh_containsk(openvcd_mvar, child1->child_variables, "003"));
	should_be_true(kh_containsk(openvcd_mvar, child2->child_variables, "004"));
	k = kh_get(openvcd_mscope, root->child_scopes, "child1");
	should_be_true(k != kh_end(root->child_scopes));
	should_equal(child1, kh_val(root->child_scopes, k));
	k = kh_get(openvcd_mscope, root->child_scopes, "child2");
	should_be_true(k != kh_end(root->child_scopes));
	should_equal(child2, kh_val(root->child_scopes, k));
	k = kh_get(openvcd_mvar, root->child_variables, "001");
	should_be_true(k != kh_end(root->child_variables));
	should_equal(root_var1, kh_val(root->child_variables, k));
	k = kh_get(openvcd_mvar, root->child_variables, "002");
	should_be_true(k != kh_end(root->child_variables));
	should_equal(root_var2, kh_val(root->child_variables, k));
	k = kh_get(openvcd_mvar, root->child_variables, "003");
	should_be_false(k != kh_end(root->child_variables));
	k = kh_get(openvcd_mvar, root->child_variables, "004");
	should_be_false(k != kh_end(root->child_variables));
	k = kh_get(openvcd_mvar, child1->child_variables, "003");
	should_be_true(k != kh_end(child1->child_variables));
	should_equal(child1_var3, kh_val(child1->child_variables, k));
	k = kh_get(openvcd_mvar, child2->child_variables, "004");
	should_be_true(k != kh_end(child2->child_variables));
	should_equal(child2_var4, kh_val(child2->child_variables, k));
	k = kh_get(openvcd_mvar, child1->child_variables, "001");
	should_be_false(k != kh_end(child1->child_variables));
	k = kh_get(openvcd_mvar, child1->child_variables, "002");
	should_be_false(k != kh_end(child1->child_variables));
	k = kh_get(openvcd_mvar, child1->child_variables, "004");
	should_be_false(k != kh_end(child1->child_variables));
	k = kh_get(openvcd_mvar, child2->child_variables, "001");
	should_be_false(k != kh_end(child2->child_variables));
	k = kh_get(openvcd_mvar, child2->child_variables, "002");
	should_be_false(k != kh_end(child2->child_variables));
	k = kh_get(openvcd_mvar, child2->child_variables, "003");
	should_be_false(k != kh_end(child2->child_variables));

	/* Now test the tricky case -- where we free a child but not it's
	 * parent. Valgrind will check for leaks if the child variables are
	 * not free-ed correctly. */
	openvcd_free_scope(child2);
	k = kh_get(openvcd_mscope, root->child_scopes, "child2");
	should_be_true(k == kh_end(root->child_scopes));

	openvcd_free_scope(root);

}


int main(void) {
	test_reference();
	test_var();
	test_scope();
	return 0;
}
