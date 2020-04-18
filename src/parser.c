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
	p->current_token = NULL;
	p->next_token = NULL;
	p->lineno = 0;

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
	if (p->current_token != NULL){ openvcd_free_token(p->current_token); }
	if (p->next_token != NULL){ openvcd_free_token(p->next_token); }
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

	if (p->state == OPENVCD_PARSER_STATE_EOF) {
		return NULL;
	}

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

				p->state = OPENVCD_PARSER_STATE_ERROR;
				p->error = OPENVCD_ERROR_ALLOC_FAILED;
				asprintf(&(p->error_string),
					"failed to allocate space to read next token");
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
		p->state = OPENVCD_PARSER_STATE_ERROR;
		p->error = OPENVCD_ERROR_TOKEN;
		asprintf(&(p->error_string),
			"failed to create token from text '%s' of length %ld",
			read_text,
			text_size );
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
				if (p->cursor == '\n') { p->lineno ++; }
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

void openvcd_advance(openvcd_parser* p) {
	if (p->state != OPENVCD_PARSER_STATE_RUNNING) {
		/* presumably we have reached EOF or have an error that is
		 * not handled yet. */
		return;
	}

	if (p->current_token != NULL) {
		openvcd_free_token(p->current_token);
	}
	p->current_token = p->next_token;
	p->next_token = openvcd_next_token(p);
}

char* openvcd_parse_until(openvcd_parser* p, char* until, char* type) {
	char* text;
	char* temp;


	text = NULL;

	while(!openvcd_token_eq_str(p->next_token, until)) {
		openvcd_advance(p);

		if ((p->state == OPENVCD_PARSER_STATE_EOF) && \
			(!openvcd_token_eq_str(p->next_token, until))) {

			p->state = OPENVCD_PARSER_STATE_ERROR;
			p->error = OPENVCD_ERROR_SYNTAX;
			asprintf(&(p->error_string),
				"syntax error on line %lu, got EOF while parsing %s",
				p->lineno,
				type);
			return NULL;
		}

		if ((p->state != OPENVCD_PARSER_STATE_RUNNING) && \
			(!openvcd_token_eq_str(p->next_token, until))) {
			return NULL;
		}

		if (p->current_token != NULL) {
			if (text == NULL) {
				text = strndup(
						p->current_token->literal,
						p->current_token->length);
			} else {
				asprintf(&temp, "%s %s",
						text,
						p->current_token->literal);
				free(text);
				text = temp;
			}
		}

		if (p->state == OPENVCD_PARSER_STATE_EOF) { break; }
	}

	return text;
}

void openvcd_parse(openvcd_parser* p) {
	p->current_token = NULL;
	p->next_token = openvcd_next_token(p);

	while(p->state == OPENVCD_PARSER_STATE_RUNNING) {

		if (openvcd_token_eq_str(p->next_token, "$version")) {
			openvcd_parse_version(p);
		}

		openvcd_advance(p);

	}

}

bool openvcd_token_eq_str(openvcd_token* t, char* s) {
	return (strncmp(t->literal, s, t->length) == 0);
}

char* openvcd_parse_version(openvcd_parser* p) {

	/* consume $version */
	openvcd_advance(p);

	char* s = openvcd_parse_until(p, "$end", "$version");

	/* consume $end */
	openvcd_advance(p);

	return s;
}

char* openvcd_parse_date(openvcd_parser* p) {
	/* consume $date*/
	openvcd_advance(p);

	char* s = openvcd_parse_until(p, "$end", "$date");

	/* consume $end */
	openvcd_advance(p);

	return s;
}

openvcd_timescale openvcd_parse_timescale(openvcd_parser* p) {
	openvcd_timescale ts;
	char* tsstring;
	char* unitstr;
	char* numstr;
	bool validating_unit;
	bool no_timescale;

	ts.u = openvcd_unit_undefined;
	ts.n = -1;

	/* consume $timescale*/
	openvcd_advance(p);

	tsstring = openvcd_parse_until(p, "$end", "$timescale");

	if ((p->state != OPENVCD_PARSER_STATE_RUNNING) &&
		(p->state != OPENVCD_PARSER_STATE_EOF))  {
		return ts;
	}

	/* validate the timescale string and throw a syntax error if it fails
	 * */
	validating_unit = false;
	for (size_t i = 0 ; i < strlen(tsstring) ; i++) {
		if (validating_unit) {
			if ( (tsstring[i] != 's') && \
			     (tsstring[i] != 'm') && \
			     (tsstring[i] != 'u') && \
			     (tsstring[i] != 'n') && \
			     (tsstring[i] != 'p') && \
			     (tsstring[i] != 'f') ) {

				/* syntax error ! */

				p->state = OPENVCD_PARSER_STATE_ERROR;
				p->error = OPENVCD_ERROR_SYNTAX;
				asprintf(&(p->error_string),
					"syntax error on line %lu, invalid character '%c' in timescale",
					p->lineno, tsstring[i]);

				free(tsstring);

				return ts;

			}
		} else {
			if ( (tsstring[i] != '0') && (tsstring[i] != '1') ) {
				i--;
				validating_unit = true;
			}
		}
	}

	unitstr = openvcd_charfilter(tsstring, "smunpf");
	numstr = openvcd_charfilter(tsstring, "10");

	no_timescale = false;

	/* #lizard forgives the complexity */
	if (strcmp(unitstr, "s") == 0)       { ts.u = openvcd_unit_s;  goto unit_ok; }
	else if (strcmp(unitstr, "ms") == 0) { ts.u = openvcd_unit_ms; goto unit_ok; }
	else if (strcmp(unitstr, "us") == 0) { ts.u = openvcd_unit_us; goto unit_ok; }
	else if (strcmp(unitstr, "ns") == 0) { ts.u = openvcd_unit_ns; goto unit_ok; }
	else if (strcmp(unitstr, "ps") == 0) { ts.u = openvcd_unit_ps; goto unit_ok; }
	else if (strcmp(unitstr, "fs") == 0) { ts.u = openvcd_unit_fs; goto unit_ok; }
	no_timescale = true; /* goto should prevent this unless there was an error */

unit_ok:
	if (strcmp(numstr, "1") == 0)        { ts.n = 1;   goto num_ok; }
	else if (strcmp(numstr, "10") == 0)  { ts.n = 10;  goto num_ok; }
	else if (strcmp(numstr, "100") == 0) { ts.n = 100; goto num_ok; }
	no_timescale = true;

num_ok:

	if (no_timescale) {
		p->state = OPENVCD_PARSER_STATE_ERROR;
		p->error = OPENVCD_ERROR_SYNTAX;
		asprintf(&(p->error_string),
			"syntax error on line %lu, invalid timescale '%s'",
			p->lineno, tsstring);
	}

	free(unitstr);
	free(numstr);
	free(tsstring);


	/* consume $end */
	openvcd_advance(p);

	return ts;

}
