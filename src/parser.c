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
	char* read_text;
	unsigned long pos;
	size_t text_size;
	openvcd_token* t;
	char* temp;

	read_text = malloc(sizeof(char) * 10);
	if (read_text == NULL) {return NULL;}
	read_text[0] = '\0';
	pos = 0;
	text_size = 10;


	/* ensure there is at least one character ready to go, and also eat
	 * whitespace */
	do { openvcd_next_char(p); } while (OPENVCD_IS_WHITESPACE(p->cursor));

	while (p->state != OPENVCD_PARSER_STATE_EOF) {
		if (OPENVCD_IS_WHITESPACE(p->cursor)) { break; }

		read_text[pos] = p->cursor;
		read_text[pos+1] = '\0';
		pos++;

		/* resize the text buffer if we run out of room */
		if ((pos + 2) > text_size) {
			text_size *= 2;
			temp = realloc(read_text, text_size);

			if (temp == NULL) {
				free(read_text);
				return NULL;
			}

			read_text = temp;
			temp = NULL;
		}

		openvcd_next_char(p);
	}

	t = openvcd_new_tokenn(read_text, text_size);
	free(read_text);
	if (t == NULL) {
		return NULL;
	}

	return t;
}

void openvcd_next_char(openvcd_parser* p) {
	/* we were just initialized */
	if (p->state == OPENVCD_PARSER_STATE_INITIALIZED) {
		p->state = OPENVCD_PARSER_STATE_RUNNING;
		p->position = 0;

		if (p->type == OPENVCD_PARSER_STRING) {
			/* case where we get called on an empty string */
			if (p->input_length < 1) {
				p->state = OPENVCD_PARSER_STATE_EOF;
				p->cursor = '\0';
			} else {
				p->cursor = p->source.input_string[0];
			}

		} else {
			fprintf(stderr, "READING FROM FILE NOT IMPLEMENTED YET\n");
			exit(1);
		}
	} else {
		p->position++;
		if (p->type == OPENVCD_PARSER_STRING) {
			if (p->position >= p->input_length) {
				p->state = OPENVCD_PARSER_STATE_EOF;
				p->cursor = '\0';
			} else {
				p->cursor = p->source.input_string[p->position];
			}

		} else {
			fprintf(stderr, "READING FROM FILE NOT IMPLEMENTED YET\n");
			exit(1);
		}

	}
}
