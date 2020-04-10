/* Copyright 2020 Charles Daniels
 *
 * This file is part of OpenVCD and is released under a BSD 3-clause license.
 * See the LICENSE file in the project root for more information */

#include "parser.h"

openvcd_parser* openvcd_new_parser(openvcd_parser_type type, openvcd_input_source source, size_t input_length) {
	openvcd_parser* p;

	p = malloc(sizeof(openvcd_parser));
	if (p == NULL) { return NULL; }

	p->state = OPENVCD_PARSER_STATE_INITIALIZED;
	p->error = OPENVCD_ERROR_NONE;
	p->type = type;
	p->source = source;
	p->input_length = input_length;
	p->position = 0;
	p->cursor = '\0';
	p->error_string = NULL;

	if ((input_length == 0) && (type != OPENVCD_PARSER_FILE)) {
		p->state = OPENVCD_PARSER_STATE_ERROR;
		p->error = OPENVCD_ERROR_NO_LENGTH;
		asprintf( &(p->error_string),
			"Parser allocated with non-stream input type, but no length provided.");
	}
	
	return p;
}

void openvcd_free_parser(openvcd_parser* p) {
	openvcd_clear_error(p);
	free(p);
}

void openvcd_clear_error(openvcd_parser* p) {
	p->error = OPENVCD_ERROR_NONE;
	if (p->state == OPENVCD_PARSER_STATE_ERROR) {
		free(p->error_string);
		p->state = OPENVCD_PARSER_STATE_UNINITIALIZED;
	}
	p->error_string = NULL;
}

openvcd_token* openvcd_new_token(char* literal) {
	openvcd_token* t;

	t = malloc(sizeof(openvcd_token));
	if (t == NULL) { return NULL; }

	t->literal = strdup(literal);
	t->length = strlen(literal);

	return t;
}


openvcd_token* openvcd_new_tokenn(char* literal, size_t length) {
	openvcd_token* t;

	t = malloc(sizeof(openvcd_token));
	if (t == NULL) { return NULL; }

	t->literal = strndup(literal, length);
	t->length = length;

	return t;
}

void openvcd_free_token(openvcd_token* t) {
	free(t->literal);
	free(t);
}

openvcd_token* openvcd_next_token(openvcd_parser* p) {
	return openvcd_new_token("TODO");
}
