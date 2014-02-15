/* Licensed under LGPL - see LICENSE file for details */
#include <ccan/tok_stream/tok_stream.h>

void tok_stream_init(struct tok_stream *ts, char delim) {
	ts->partial_tok = NULL;
	ts->partial_tok_len = 0;
	ts->delim = delim;
}

bool tok_stream_free(struct tok_stream *ts, char **remainder) {
	if(ts->partial_tok != NULL) {
		if(remainder == NULL)
			free(ts->partial_tok);
		else
			*remainder = ts->partial_tok;

		return true;
	}

	return false;
}

/* add the current value returned by tok_itr_val to *base and
 * change *len to the length of the string to which *base points.
 * if *base is not null, *len must be the length of the string at
 * *base. this function assumes tok_itr_end(&ts->itr) == 0.
 */
static int add_val(const struct tok_stream *ts, char **base, size_t *len) {
	size_t offset, val_len;
	char *tmp;

	/* the length of the current token.
	 * add one for the null byte. */
	val_len = tok_itr_val_len(&ts->itr) + 1;
	if(*base == NULL || *len < 1) {
		/* if *base is empty, just allocate space for the
		 * current token. */
		offset = 0;
		*base = malloc(sizeof(char) * val_len);
		if(*base == NULL)
			return -1;
	}
	else {
		/* *base already has some data (a previous partial
		 * token), so read the length of the current string
		 * at *base from len and subtract 1 b.c. we will
		 * overwrite the null byte that terminates base. */
		offset = *len - 1;

		tmp = realloc(*base, sizeof(char)*(offset + val_len));
		if(tmp == NULL)
			return -1;

		*base = tmp;
	}

	/* copy the current token to the end of the string
	 * at *base. */
	tok_itr_val(&ts->itr, *base + offset, val_len);
	*len = offset + val_len;
	return 0;
}

int tok_stream(struct tok_stream *ts, char **token) {
	size_t len;

	if(tok_itr_end(&ts->itr))
		return -1;

	/* if this is true then this is the last
	 * token AND its a partial token (not empty),
	 * so we return NULL and save this partial
	 * token in ts->partial_tok */
	if(tok_itr_partial_val(&ts->itr) ) {
		/* add the current tok_itr value to the current
		 * partial_tok value */
		if(add_val(ts, &ts->partial_tok, &ts->partial_tok_len) != 0)
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

	if(add_val(ts, token, &len) != 0)
		return -2;

	ts->partial_tok = NULL;
	tok_itr_next(&ts->itr);
	return 0;
} /* tok_stream */
