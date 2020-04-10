/* Copyright 2020 Charles Daniels
 *
 * This file is part of OpenVCD and is released under a BSD 3-clause license.
 * See the LICENSE file in the project root for more information */

#define _GNU_SOURCE
#include "stdio.h"

#include "test_util.h"
#include "parser.h"

void test_lexing(void) {
	openvcd_parser* p;
	openvcd_token* t;
	openvcd_input_source s;
	char* lexing_test_input;

	asprintf(&lexing_test_input, "tok1 $tok2 XXXtok3 tok4!__ 1234tok5\ntok6\ttok7 looooooooonnnnngggggggggggtok8");

	/* these make the C compiler sad if you put them in as literals, since
	 * it tries to expand the backslash and gets confused  */
	lexing_test_input[11] = 0x5c; /* backslash */
	lexing_test_input[12] = 0x7e; /* tilde */
	lexing_test_input[13] = 0x60; /* grave */

	s.input_string = lexing_test_input;
	p = openvcd_new_parser(OPENVCD_PARSER_STRING, s, strlen(lexing_test_input));

	t = openvcd_next_token(p);
	str_should_equal(t->literal, "tok1");
	openvcd_free_token(t);

	t = openvcd_next_token(p);
	str_should_equal(t->literal, "$tok2");
	openvcd_free_token(t);

	t = openvcd_next_token(p);
	str_should_equal(t->literal, "\\~`tok3");
	should_equal(t->literal[0], 0x5c);
	should_equal(t->literal[1], 0x7e);
	should_equal(t->literal[2], 0x60);
	openvcd_free_token(t);

	t = openvcd_next_token(p);
	str_should_equal(t->literal, "tok4!__");
	openvcd_free_token(t);

	t = openvcd_next_token(p);
	str_should_equal(t->literal, "1234tok5");
	openvcd_free_token(t);

	t = openvcd_next_token(p);
	str_should_equal(t->literal, "tok6");
	openvcd_free_token(t);

	t = openvcd_next_token(p);
	str_should_equal(t->literal, "tok7");
	openvcd_free_token(t);

	t = openvcd_next_token(p);
	str_should_equal(t->literal, "looooooooonnnnngggggggggggtok8");
	openvcd_free_token(t);

	openvcd_free_parser(p);

	free(lexing_test_input);
}

void test_init(void) {
	openvcd_parser* p;
	openvcd_input_source s;
	char* init_test_input;

	asprintf(&init_test_input, "abcdefg");

	/* common case */
	s.input_string = init_test_input;
	p = openvcd_new_parser(OPENVCD_PARSER_STRING, s, strlen(init_test_input));
	should_not_equal(p->state, OPENVCD_PARSER_STATE_ERROR);
	openvcd_free_parser(p);

	/* ensure string length provided */
	s.input_string = init_test_input;
	p = openvcd_new_parser(OPENVCD_PARSER_STRING, s, 0);
	should_equal(p->state, OPENVCD_PARSER_STATE_ERROR);
	should_equal(p->error, OPENVCD_ERROR_NO_LENGTH);
	should_not_be_null(p->error_string);
	openvcd_clear_error(p);
	should_be_null(p->error_string);
	should_equal(p->state, OPENVCD_PARSER_STATE_UNINITIALIZED);
	should_equal(p->error, OPENVCD_ERROR_NONE);
	openvcd_free_parser(p);

	free(init_test_input);
}

int main(void) {
	test_init();
	test_lexing();
}
