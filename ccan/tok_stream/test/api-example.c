#include <ccan/tap/tap.h>
#include <ccan/tok_stream/tok_stream.h>
#include <ccan/array_size/array_size.h>

int main(int argc, char *argv[]) {
	const char *tokens[] = {
		"A", "black", "hole", "is", "a", "region", "of", "spacetime", "from", 
		"which", "gravity", "prevents", "anything,", "including", "light,", 
		"from", "escaping.[1]", "The", "theory", "of", "general", "relativity",
		"predicts", "that", "a", "sufficiently", "compact", "mass", "will", 
		"deform","spacetime", "to", "form", "a", "black", "hole.", "Around", 
		"a", "black", "hole", "there", "is", "a", "mathematically", "defined", 
		"surface", "called", "an", "event", "horizon", "that", "marks", "the",
		"point", "of", "no", "return.", "It", "is", "called", "\"black\"",
		"because", "it", "absorbs", "all", "the", "light", "that", "hits",
		"the", "horizon,", "reflecting", "nothing,", "just", "like", "a",
		"perfect", "black", "body", "in", "thermodynamics.[2][3]", "Quantum",
		"field", "theory", "in", "curved", "spacetime", "predicts", "that",
		"event", "horizons", "emit", "radiation", "like", "a", "black", "body",
		"with", "a", "finite", "temperature.", "This", "temperature", "is",
		"inversely", "proportional", "to", "the", "mass", "of", "the", "black",
		"hole,", "making", "it", "difficult", "to", "observe", "this",
		"radiation", "for", "black", "holes", "of", "stellar", "mass",
		"or", "greater."
	};

	const char *input[] = {
		"A black hole is a region of sp",
        "acetime from which gravity pre",
        "vents anything, including ligh",
        "t, from escaping.[1] The theor",
        "y of general relativity predic",
        "ts that a sufficiently compact",
        " mass will deform spacetime to",
        " form a black hole. Around a b",
        "lack hole there is a mathemati",
        "cally defined surface called a",
        "n event horizon that marks the",
        " point of no return. It is cal",
        "led \"black\" because it absorbs",
        " all the light that hits the h",
        "orizon, reflecting nothing, ju",
        "st like a perfect black body i",
        "n thermodynamics.[2][3] Quantu",
        "m field theory in curved space",
        "time predicts that event horiz",
        "ons emit radiation like a blac",
        "k body with a finite temperatu",
        "re. This temperature is invers",
        "ely proportional to the mass o",
        "f the black hole, making it di",
        "fficult to observe this radiat",
        "ion for black holes of stellar",
        " mass or greater."
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