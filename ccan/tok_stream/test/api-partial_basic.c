#include <ccan/tap/tap.h>
#include <ccan/tok_stream/tok_stream.h>
#include <ccan/array_size/array_size.h>

int main(int argc, char *argv[]) {
	const char *tokens[] = {
		"A", "black", "hole", "is", "a", "region", "of", "spacetime", "from",
		"whichgravitypreventsanythingincludinglightfromescaping.[1]",
		"The", "theory", "of", "general", "relativity", "predicts",
		"that", "a"
	};

	const char *input[] = {
		"A black hole is a region of sp",
		"acetime from whichgravitypreve",
		"ntsanythingincludinglightfrome",
		"scaping.[1] The theory of gene",
		"ral relativity predicts that a"
	};
	char *result_toks[512];
	int i, tok_idx;
	struct tok_stream ts;

	plan_tests(ARRAY_SIZE(tokens) );
	tok_stream_init(&ts, ' ');
	tok_idx = 0;

	for(i = 0; i < ARRAY_SIZE(input); i++) {
		tok_stream_input(&ts, input[i]);
		while(tok_stream(&ts, &result_toks[tok_idx]) == 0) {
			/*diag("token %d: %s", tok_idx, result_toks[tok_idx]);*/
			ok1(strcmp(result_toks[tok_idx], tokens[tok_idx]) == 0);
			tok_idx++;
		}
	}
	if(tok_stream_free(&ts, &result_toks[tok_idx])) {
		ok1(strcmp(result_toks[tok_idx], tokens[tok_idx]) == 0);
		tok_idx++;
	}
	else
		fail("expected a trailing token");

	for(i = 0; i < tok_idx; i++) {
		free(result_toks[i]);
	}
	return exit_status();
}
