/* Licensed under LGPL - see LICENSE file for details */
#ifndef CCAN_TOK_STREAM_H
#define CCAN_TOK_STREAM_H

#include <ccan/tok_itr/tok_itr.h>

struct tok_stream {
	struct tok_itr itr;
	const char *partial_tok;
	size_t partial_tok_len;
};

static void tok_stream_init(struct tok_stream *ts, const char *input, char delim) {
	ts->partial_tok = NULL;
	ts->partial_tok_len = 0;
	tok_itr_init(&ts->itr, input, delim);
}

static void tok_stream_input(struct tok_stream *ts, const char *input) {
	tok_itr_init(&ts->itr, input, tok_itr_delim(&ts->itr) );
}

static int add_val(const struct tok_stream *ts, char **base, size_t len) {
	size_t offset, val_len;
	char *tmp;

	val_len = tok_itr_val_len(&ts->itr); 
	if(*base == NULL) {
		offset = 0;
		*base = malloc(sizeof(char) * val_len);
		if(*base == NULL)
			return -1;
	}
	else {
		/* subtract 1 from new len b.c. we will 
		 * overwrite the null byte that terminates base. */
		offset = len - 1;

		tmp = realloc(*base, sizeof(char)*(offset + val_len));
		if(tmp == NULL)
			return -1;

		*base = tmp;
	}

	tok_itr_val(&ts->itr, *base + offset, val_len);
	return 0;
}

static int tok_stream(struct tok_stream *ts, char **token) {
	size_t len;

	if(tok_itr_end(&ts->itr))
		return -1;

	tok_itr_next(ts->itr);
	if(tok_itr_end(&ts->itr))
		return -1;

	/* if this is true then this is the last
	 * token AND its a partial token (not empty),
	 * so we return NULL and save this partial 
	 * token in ts->partial_tok */
	if(tok_itr_partial_val(&ts->itr) ) {
		if(add_val(&ts, &ts->partial_tok, ts->partial_tok_len) != 0)
			return -2;

		return -1;
	} 

	if(ts->partial_tok != NULL) {
		*token = ts->partial_tok;
		len = ts->partial_tok_len;
	}
	else {
		*token = NULL;
		len = 0;
	}

	if(add_val(&ts, token, len) != 0)
		return -2

	ts->partial_tok_len = NULL;
	return 0;
} /* tok_stream */

#endif /* CCAN_TOK_STREAM_H */

