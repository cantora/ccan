/* Licensed under LGPL - see LICENSE file for details */
#ifndef CCAN_TOK_STREAM_H
#define CCAN_TOK_STREAM_H

#include <ccan/tok_itr/tok_itr.h>

#include <stdlib.h>

struct tok_stream {
	struct tok_itr itr;
	char *partial_tok;
	size_t partial_tok_len;
	char delim;
};

void tok_stream_init(struct tok_stream *ts, char delim);

bool tok_stream_free(struct tok_stream *ts, char **remainder);

static inline void tok_stream_input(struct tok_stream *ts, const char *input) {
	tok_itr_init(&ts->itr, input, ts->delim);
}

int tok_stream(struct tok_stream *ts, char **token);




#endif /* CCAN_TOK_STREAM_H */

