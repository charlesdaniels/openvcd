/* Copyright 2020 Charles Daniels
 *
 * This file is part of OpenVCD and is released under a BSD 3-clause license.
 * See the LICENSE file in the project root for more information */

/**** OVERVIEW ***************************************************************/

/*
 * This file implements a lexer/parser for Verilog VCD according to IEEE Std.
 * 1800-2012. It implements only 4-state VCD files, not extended VCD.
 *
 * The lexer is built in with the parser because it is very simple. Since
 * VCD identifiers can contain any printing character other than space (0x20),
 * there isn't really any meaningful lexical analysis that can be done. Thus
 * the lexer implemented here is a very simple one that merely accumulated
 * tokens delineated by space characters.
 *
 * The parser is a bespoke recursive descent parser. This approach was chosen
 * in lieu of using a parser generator because VCD is very straightforward
 * format, and a bespoke parser both allows for more useful error information
 * as well as eliminates a dependency on an external parser generator.
 */

/**** INCLUDES ***************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "vec.h"

/**** TYPES ******************************************************************/

/* Determines if the parser state object corresponds to a file stream or a
 * string. */
typedef enum {OPENVCD_PARSER_FILE, OPENVCD_PARSER_STRING} openvcd_parser_type;

/* Encodes possible input sources that the parser might draw from */
typedef union{
	char* input_string;
	FILE* input_stream;
} openvcd_input_source;

typedef struct {
	char* literal;
	size_t length;
} openvcd_token;

typedef enum {

	/* the parser is in an uninitialized, inconsistent, or unknown state */
	OPENVCD_PARSER_STATE_UNINITIALIZED=0,

	/* the parser has encountered an error */
	OPENVCD_PARSER_STATE_ERROR,

	/* when the parser has just been initialized but not read anything yet
	 * */
	OPENVCD_PARSER_STATE_INITIALIZED,

	/* the parser is running and has not encountered EOF yet */
	OPENVCD_PARSER_STATE_RUNNING,

	/* the parser has encountered EOF and has finished parsing the input */
	OPENVCD_PARSER_STATE_EOF
} openvcd_parser_state;

typedef enum {
	OPENVCD_ERROR_NONE=0,
	OPENVCD_ERROR_GENERAL,

	/* This error condition signals that a parser source has been selected
	 * which requires an input length, but none (e.g. 0) has been provided.
	 */
	OPENVCD_ERROR_NO_LENGTH,

	/* An error occurred while attempting to read a new token */
	OPENVCD_ERROR_TOKEN,

	/* An attempt to allocate memory failed */
	OPENVCD_ERROR_ALLOC_FAILED,

	/* A syntax error occured while parsing */
	OPENVCD_ERROR_SYNTAX,
} openvcd_parser_error;

#define OPENVCD_PARSER_ERROR_TO_STR(_err) \
	(_err == OPENVCD_ERROR_NONE) ? "NONE" : \
	(_err == OPENVCD_ERROR_GENERAL) ? "GENERAL" : \
	(_err == OPENVCD_ERROR_NO_LENGTH) ? "NO LENGTH" : \
	(_err == OPENVCD_ERROR_TOKEN) ? "TOKEN" : \
	(_err == OPENVCD_ERROR_SYNTAX) ? "SYNTAX" : \
	(_err == OPENVCD_ERROR_ALLOC_FAILED) ? "ALLOC FAILED" : "UNKNOWN ERROR"

typedef struct {

	/* this is used to determine what state the parser is in */
	openvcd_parser_state state;

	/* provides an error type of the parser is in the state
	 * OPENVCD_PARSER_STATE_ERROR, has no meaning otherwise */
	openvcd_parser_error error;

	/* determine what type input_source is */
	openvcd_parser_type type;

	/* the source of characters for the lexer */
	openvcd_input_source source;

	/* Strings must provide an input length, but streams do not. Streams
	 * may provide an input length if they wish the parser to halt after a
	 * given number of bytes. If the input is a stream and the input length
	 * is 0, there is assumed to be no limit. It is safe to set this value
	 * longer than an input strings input length if and only if the string
	 * is null terminated. */
	size_t input_length;

	/* number of characters read so far */
	unsigned long position;

	/* current line number, used for error messages */
	unsigned long lineno;

	/* the character we are lexing right now */
	char cursor;

	/* this is only safe to read if the parser state is
	 * OPENVCD_PARSER_STATE_ERROR */
	char* error_string;

	/* used during parsing, null if not available or uninitialized */
	openvcd_token* current_token;
	openvcd_token* next_token;

} openvcd_parser;

/**** UTILITIES **************************************************************/

#define OPENVCD_IS_WHITESPACE(_ch) ( ((_ch) == ' ') || ((_ch) == '\t') || ((_ch) == '\n') || ((_ch) == '\r') )

/**** PROTOTYPES *************************************************************/

/**
 * @brief Allocate a new OpenVCD parser object.
 *
 * The parser will be allocated on the heap with malloc(), and
 * openvcd_free_parser() must be used later to free the parser.
 *
 * The caller should check if the parser is in an error state before using it.
 *
 * Note that the caller must manage the lifecycle of the input string if
 * the parser type is OPENVCD_PARSER_STRING. The caller must free() it
 * independently of openvcd_free_parser(). Likewise, stream input types
 * will not be closed or otherwise cleaned up when the parser is free-ed.
 *
 * @param type either OPENVCD_PARSER_FILE or OPENVCD_PARSER_STRING
 * @param source either a FILE* or a char* depending on the parser type
 * @param input_length the input length for stream types only if no limit is
 * desired 0 may be used. For string, may be set higher than the string length
 * if desired.
 *
 * @return The new parser, or NULL on failure.
 */
openvcd_parser* openvcd_new_parser(openvcd_parser_type type, openvcd_input_source source, size_t input_length);

/**
 * @brief Free-es a previously allocated OpenVCD parser object.
 *
 * Note that if the parser is in state OPENVCD_PARSER_STATE_ERROR, this
 * function will also de-allocate the error_string field using
 * openvcd_clear_error().
 *
 * @param p
 */
void openvcd_free_parser(openvcd_parser* p);

/**
 * @brief Clear the parser error, if any.
 *
 * This function will clear the error and error_string fields, and reset the
 * parser state to OPENVCD_PARSER_STATE_UNINITIALIZED. The caller is expected
 * to do any needed error handling, which will usually be exiting with an
 * error.
 *
 * If the parser is not in an error state, this function will simply reset
 * error_string to NULL and the error field to OPENVCD_ERROR_NONE. This may
 * cause a memory leak if there is an un-free-ed value in error_string.
 *
 * @param p
 */
void openvcd_clear_error(openvcd_parser* p);

/**
 * @brief Allocate a new token.
 *
 * Note that the given literal will be duplicated internally into a new heap
 * allocated string. The caller must manage the lifecycle of literal
 * appropriately.
 *
 * The literal *must* be null terminated. When possible openvcd_new_tokenn()
 * should be used instead.
 *
 * @param literal
 *
 * @return The new token or NULL if allocation failed.
 */
openvcd_token* openvcd_new_token(char* literal);

/**
 * @brief Allocate a new token of known length.
 *
 * This is the preferred way to allocate a token. it works similarly to
 * openvcd_new_token(), but explicitly accepts the length rather than
 * relying on strlen().
 *
 * Again, remember that literal is duplicated and must be free-ed by the
 * caller.
 *
 * @param literal
 * @param length
 *
 * @return The new token or NULL if allocation failed.
 */
openvcd_token* openvcd_new_tokenn(char* literal, size_t length);

/**
 * @brief Free a previously allocated token.
 *
 * @param openvcd_token
 */
void openvcd_free_token(openvcd_token* t);

/**
 * @brief Advance the parser appropriately until a new token is fully read.
 *
 * When this function returns, cursor and position will point to the space
 * character immediately after the token, or will be NULL and point beyond
 * the end of the file if EOF is reached.
 *
 * @param p
 *
 * @return
 */
openvcd_token* openvcd_next_token(openvcd_parser* p);

/**
 * @brief Advance the parser by one character.
 *
 * @param p
 */
void openvcd_next_char(openvcd_parser* p);

/**
 * @brief Parse the entire input.
 *
 * @param p
 */
void openvcd_parse(openvcd_parser* p);

/**
 * @brief Advance the parser by one token.
 *
 * @param p
 */
void openvcd_advance(openvcd_parser* p);

/**
 * @brief Parse a version header.
 *
 * @param p
 *
 * @return The version text in a newly malloc-ed buffer, which the caller
 * should free appropriate.
 */
char* openvcd_parse_version(openvcd_parser* p);

/**
 * @brief Return true if the token and string match.
 *
 * @param t
 * @param s
 *
 * @return
 */
bool openvcd_token_eq_str(openvcd_token* t, char* s);
