#include "../tok_stream.c"
#include <ccan/tap/tap.h>

static void test_add_val_with(const char *input, const char *first_token) {
	struct tok_stream stream;
	size_t len, first_token_len;
	char *str, *double_token;

	first_token_len = strlen(first_token);
	double_token = malloc(sizeof(char)*first_token_len*2 + 1);
	if(double_token == NULL)
		return;
	strncpy(double_token, first_token, first_token_len);
	strncpy(double_token+first_token_len, first_token, first_token_len+1);

	tok_stream_init(&stream, ' ');
	tok_stream_input(&stream, input);

	/* *base is properly empty */
	len = 0;
	str = NULL;
	ok1(tok_itr_end(&stream.itr) == 0);
	ok1(add_val(&stream, &str, &len) == 0);
	ok1(strcmp(str, first_token) == 0);
	ok1(len == first_token_len+1);
	ok1(strlen(str) == first_token_len);
	free(str);

	/* *base is empty, but length is wrong */
	len = 23;
	str = NULL;
	ok1(tok_itr_end(&stream.itr) == 0);
	ok1(add_val(&stream, &str, &len) == 0);
	ok1(strcmp(str, first_token) == 0);
	ok1(len == first_token_len+1);
	ok1(strlen(str) == first_token_len);
	free(str);

	/* *base is non-null, but length is 0 */
	len = 0;
	str = (char *) 0xbadf00d;
	ok1(tok_itr_end(&stream.itr) == 0);
	ok1(add_val(&stream, &str, &len) == 0);
	ok1(strcmp(str, first_token) == 0);
	ok1(len == first_token_len+1);
	ok1(strlen(str) == first_token_len);
	/* no free, append next time */

	/* *base is non-empty */
	ok1(tok_itr_end(&stream.itr) == 0);
	ok1(add_val(&stream, &str, &len) == 0);
	ok1(strcmp(str, double_token) == 0);
	ok1(len == first_token_len*2 + 1);
	ok1(strlen(str) == first_token_len*2);
	free(str);
}

int main(int argc, char *argv[]) {
	plan_tests(5*4);

	test_add_val_with("hey", "hey");
	return exit_status();
}
