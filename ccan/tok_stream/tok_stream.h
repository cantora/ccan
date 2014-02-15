/* Licensed under LGPL - see LICENSE file for details */
#ifndef CCAN_TOK_STREAM_H
#define CCAN_TOK_STREAM_H

#include <ccan/tok_itr/tok_itr.h>

#include <stdlib.h>

/**
 * struct tok_stream - a structure containing the token stream state
 * @itr: the state of the token iterator. see the CCAN tok_itr module.
 * @partial_tok: a partial token to be completed by the next call
 *               to tok_stream_input. this will be NULL if no token has
 *               been split across inputs.
 * @partial_tok_len: length of the partial token. this is 0 if
 *                   @partial_tok is NULL.
 * @delim: the token delimiter.
 */
struct tok_stream {
	struct tok_itr itr;
	char *partial_tok;
	size_t partial_tok_len;
	char delim;
};

/**
 * tok_stream_init - initialize the token stream struct with a delimiter.
 * @ts: a pointer to the tok_stream struct to be initialized.
 * @delim: the token delimiter.
 *
 * Initializes the tok_stream struct to stream tokens.
 */
void tok_stream_init(struct tok_stream *ts, char delim);

/**
 * tok_stream_free - free the resources associated with a token stream.
 * @ts: the tok_stream struct.
 * @remainder: an output variable. *@remainder will point to the partial
 *             token that remains. if there is no remaining partial token
 *             then this *@remainder will be NULL.
 *
 * Frees the resources associated with the token stream. If there exists
 * a remaining partial token, then *@remainder will point to it. The caller
 * will be responsible for freeing the data at *@remainder. If @remainder
 * is NULL, the partial token remainder will be thrown away.
 */
bool tok_stream_free(struct tok_stream *ts, char **remainder);

/**
 * tok_stream_input - add input string to be processed for tokens
 * @ts: a pointer to the tok_stream struct.
 * @input: a null-terminated character string to process for tokens.
 *
 * Adds the characters at @input to the token stream for processing. It
 * only makes sense to call this function after initialization or
 * after tok_stream(...) returns non-zero.
 */
static inline void tok_stream_input(struct tok_stream *ts, const char *input) {
	tok_itr_init(&ts->itr, input, ts->delim);
}

/**
 * tok_stream - get the next token in the token stream
 * @ts: a pointer to the tok_stream struct
 * @token: an output variable. this will point to the next token in the
 *         token stream if tok_stream returns 0.
 *
 * If this function returns 0, *@token points to the next token in the
 * stream. The caller is responsible for freeing the data at which
 * *@token points.
 */
int tok_stream(struct tok_stream *ts, char **token);

#endif /* CCAN_TOK_STREAM_H */

