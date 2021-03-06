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

struct test_timescale_parsing_data {
	char* input_string;
	openvcd_timescale expected_timescale;
};

void test_timescale_parsing(void) {

	static struct test_timescale_parsing_data tests[] = {
		{"$timescale 1s $end",    {.n = 1,   .u = openvcd_unit_s}},
		{"$timescale 10s $end",   {.n = 10,  .u = openvcd_unit_s}},
		{"$timescale 100s $end",  {.n = 100, .u = openvcd_unit_s}},
		{"$timescale 1ms $end",   {.n = 1,   .u = openvcd_unit_ms}},
		{"$timescale 10ms $end",  {.n = 10,  .u = openvcd_unit_ms}},
		{"$timescale 100ms $end", {.n = 100, .u = openvcd_unit_ms}},
		{"$timescale 1us $end",   {.n = 1,   .u = openvcd_unit_us}},
		{"$timescale 10us $end",  {.n = 10,  .u = openvcd_unit_us}},
		{"$timescale 100us $end", {.n = 100, .u = openvcd_unit_us}},
		{"$timescale 1ns $end",   {.n = 1,   .u = openvcd_unit_ns}},
		{"$timescale 10ns $end",  {.n = 10,  .u = openvcd_unit_ns}},
		{"$timescale 100ns $end", {.n = 100, .u = openvcd_unit_ns}},
		{"$timescale 1ps $end",   {.n = 1,   .u = openvcd_unit_ps}},
		{"$timescale 10ps $end",  {.n = 10,  .u = openvcd_unit_ps}},
		{"$timescale 100ps $end", {.n = 100, .u = openvcd_unit_ps}},
		{"$timescale 1fs $end",   {.n = 1,   .u = openvcd_unit_fs}},
		{"$timescale 10fs $end",  {.n = 10,  .u = openvcd_unit_fs}},
		{"$timescale 100fs $end", {.n = 100, .u = openvcd_unit_fs}},
		{NULL, {0, 0}}
	};

	for (int i = 0 ; tests[i].input_string != NULL ; i++) {
		openvcd_parser *p;
		openvcd_timescale ts;
		openvcd_input_source s;


		s.input_string = tests[i].input_string,
		p = openvcd_new_parser(OPENVCD_PARSER_STRING,
				s,
				strlen(tests[i].input_string));
		p->current_token = openvcd_next_token(p);

		ts = openvcd_parse_timescale(p);

		check_parser_error(p);
		should_equal(ts.n, tests[i].expected_timescale.n);
		should_equal(ts.u, tests[i].expected_timescale.u);

		openvcd_free_parser(p);
	}

	/* now test somet things that should cause errors... */
	static char* errors[] = {
		"$timescale s $end",
		"$timescale 1s",
		"$timescale 1sa $end",
		NULL,
	};

	for (int i = 0 ; errors[i] != NULL ; i++) {
		openvcd_parser *p;
		openvcd_input_source s;


		s.input_string = errors[i],
		p = openvcd_new_parser(OPENVCD_PARSER_STRING,
				s,
				strlen(tests[i].input_string));
		p->current_token = openvcd_next_token(p);

		openvcd_parse_timescale(p);


		if (p->state != OPENVCD_PARSER_STATE_ERROR) {
			printf("considering input string: %s\n", errors[i]);
			fail("parser should be in an error state but is not!%s\n", "");
		}

		openvcd_free_parser(p);
	}



}

void test_parsing(void) {
	openvcd_parser* p;
	openvcd_input_source s;
	char* parsing_test_input;

	parsing_test_input = ""
		"$version Generated by VerilatedVcd $end\n"
		"$date Sun Mar 29 11:41:13 2020\n"
		" $end\n"
		"$timescale   1ns $end\n"
		"\n"
		" $scope module TOP $end\n"
		"  $var wire  1 '\" cs $end\n"
		"  $var wire  1 ?\" miso $end\n"
		"  $var wire  1 7\" mosi $end\n"
		"  $var wire  1 /\" sck $end\n"
		" $upscope $end\n"
		"$enddefinitions $end\n"
		"\n"
		"\n"
		"#8\n"
		"1'\"\n"
		"0/\"\n"
		"07\"\n"
		"0?\"\n"
		"#10\n"
		"1/\"\n"
		"#618\n"
		"0'\"\n"
		"#698\n"
		"0/\"\n"
		"#778\n"
		"1/\"\n"
		"#858\n"
		"0/\"\n"
		"#938\n"
		"1/\"\n"
		"#1018\n"
		"0/\"\n"
		"#1098\n"
		"1/\"\n"
		"#1178\n"
		"0/\"\n"
		"#1258\n"
		"1/\"\n"
		"#1338\n"
		"0/\"\n"
		"17\"\n"
		"#1418\n"
		"1/\"\n"
		"#1498\n"
		"0/\"\n";

	s.input_string = parsing_test_input;

	/* test that advance works */
	p = openvcd_new_parser(OPENVCD_PARSER_STRING, s, strlen(parsing_test_input));
	p->current_token = openvcd_next_token(p);
	check_parser_error(p);
	p->next_token = openvcd_next_token(p);
	check_parser_error(p);
	str_should_equal(p->current_token->literal, "$version");
	str_should_equal(p->next_token->literal, "Generated");
	openvcd_advance(p);
	check_parser_error(p);
	str_should_equal(p->current_token->literal, "Generated");
	str_should_equal(p->next_token->literal, "by");
	openvcd_free_parser(p);

	/* test parsing $version */
	p = openvcd_new_parser(OPENVCD_PARSER_STRING, s, strlen(parsing_test_input));
	p->current_token = NULL;
	p->next_token = openvcd_next_token(p);
	check_parser_error(p);
	str_should_equal(p->next_token->literal, "$version");
	char* version_string = openvcd_parse_version(p);
	str_should_equal(p->next_token->literal, "$date");
	check_parser_error(p);
	should_not_be_null(version_string);
	str_should_equal(version_string, "Generated by VerilatedVcd");
	free(version_string);

	/* test parsing $date */
	str_should_equal(p->next_token->literal, "$date");
	char* date_string = openvcd_parse_date(p);
	str_should_equal(p->next_token->literal, "$timescale");
	check_parser_error(p);
	should_not_be_null(date_string);
	str_should_equal(date_string, "Sun Mar 29 11:41:13 2020");
	free(date_string);

	/* test parsing $timescale */
	str_should_equal(p->next_token->literal, "$timescale");
	openvcd_timescale ts = openvcd_parse_timescale(p);
	str_should_equal(p->next_token->literal, "$scope");
	check_parser_error(p);
	should_equal(ts.u, openvcd_unit_ns);
	should_equal(ts.n, 1);
	openvcd_free_parser(p);

	s.input_string = parsing_test_input;
	p = openvcd_new_parser(OPENVCD_PARSER_STRING, s, strlen(parsing_test_input));
	/* openvcd_parse(p); */
	check_parser_error(p);
	openvcd_free_parser(p);



}

int main(void) {
	test_init();
	test_lexing();
	test_parsing();
	test_timescale_parsing();
}
