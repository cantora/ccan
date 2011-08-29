#include <tools/ccanlint/ccanlint.h>
#include <tools/tools.h>
#include <ccan/talloc/talloc.h>
#include <ccan/str/str.h>
#include <ccan/foreach/foreach.h>
#include <ccan/grab_file/grab_file.h>
#include <ccan/str_talloc/str_talloc.h>
#include "tests_pass.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <string.h>
#include <ctype.h>

REGISTER_TEST(tests_pass_valgrind);
REGISTER_TEST(tests_pass_valgrind_noleaks);

/* Note: we already test safe_mode in run_tests.c */
static const char *can_run_vg(struct manifest *m)
{
	if (!do_valgrind)
		return talloc_asprintf(m, "No valgrind support");
	return NULL;
}

/* Example output:
==2749== Conditional jump or move depends on uninitialised value(s)
==2749==    at 0x4026C60: strnlen (mc_replace_strmem.c:263)
==2749==    by 0x40850E3: vfprintf (vfprintf.c:1614)
==2749==    by 0x408EACF: printf (printf.c:35)
==2749==    by 0x8048465: main (in /tmp/foo)
==2749== 
==2749== 1 bytes in 1 blocks are definitely lost in loss record 1 of 1
==2749==    at 0x4025BD3: malloc (vg_replace_malloc.c:236)
==2749==    by 0x8048444: main (in /tmp/foo)
==2749== 
*/

static bool blank_line(const char *line)
{
	return line[strspn(line, "=0123456789 ")] == '\0';
}

/* Removes matching lines from lines array, returns them.  FIXME: Hacky. */
static char **extract_matching(const char *prefix, char *lines[])
{
	unsigned int i, num_ret = 0;
	char **ret = talloc_array(lines, char *, talloc_array_length(lines));

	for (i = 0; i < talloc_array_length(lines) - 1; i++) {
		if (strstarts(lines[i], prefix)) {
			ret[num_ret++] = talloc_steal(ret, lines[i]);
			lines[i] = (char *)"";
		}
	}
	ret[num_ret++] = NULL;

	/* Make sure length is correct! */
	return talloc_realloc(NULL, ret, char *, num_ret);
}

static char *get_leaks(char *lines[], char **errs)
{
	char *leaks = talloc_strdup(lines, "");
	unsigned int i;

	for (i = 0; i < talloc_array_length(lines) - 1; i++) {
		if (strstr(lines[i], " lost ")) {
			/* A leak... */
			if (strstr(lines[i], " definitely lost ")) {
				/* Definite leak, report. */
				while (lines[i] && !blank_line(lines[i])) {
					leaks = talloc_append_string(leaks,
								     lines[i]);
					leaks = talloc_append_string(leaks,
								     "\n");
					i++;
				}
			} else
				/* Not definite, ignore. */
				while (lines[i] && !blank_line(lines[i]))
					i++;
		} else {
			/* A real error. */
			while (lines[i] && !blank_line(lines[i])) {
				*errs = talloc_append_string(*errs, lines[i]);
				*errs = talloc_append_string(*errs, "\n");
				i++;
			}
		}
	}
	return leaks;
}

/* Returns leaks, and sets errs[] */
static char *analyze_output(const char *output, char **errs)
{
	char *leaks = talloc_strdup(output, "");
	unsigned int i;
	char **lines = strsplit(output, output, "\n");

	*errs = talloc_strdup(output, "");
	for (i = 0; i < talloc_array_length(lines) - 1; i++) {
		unsigned int preflen = strspn(lines[i], "=0123456789");
		char *prefix, **sublines;

		/* Ignore erased lines, or weird stuff. */
		if (preflen == 0)
			continue;

		prefix = talloc_strndup(output, lines[i], preflen);
		sublines = extract_matching(prefix, lines);

		leaks = talloc_append_string(leaks, get_leaks(sublines, errs));
	}

	if (!leaks[0]) {
		talloc_free(leaks);
		leaks = NULL;
	}
	if (!(*errs)[0]) {
		talloc_free(*errs);
		*errs = NULL;
	}
	return leaks;
}

static const char *concat(struct score *score, char *bits[])
{
	unsigned int i;
	char *ret = talloc_strdup(score, "");

	for (i = 0; bits[i]; i++) {
		if (i)
			ret = talloc_append_string(ret, " ");
		ret = talloc_append_string(ret, bits[i]);
	}
	return ret;
}

/* FIXME: Run examples, too! */
static void do_run_tests_vg(struct manifest *m,
			    bool keep,
			    unsigned int *timeleft,
			    struct score *score)
{
	struct ccan_file *i;
	struct list_head *list;

	/* This is slow, so we run once but grab leak info. */
	score->total = 0;
	score->pass = true;
	foreach_ptr(list, &m->run_tests, &m->api_tests) {
		list_for_each(list, i, list) {
			char *err, *output;
			const char *options;

			score->total++;
			options = concat(score,
					 per_file_options(&tests_pass_valgrind,
							  i));
			if (streq(options, "FAIL"))
				continue;

			if (keep)
				talloc_set_destructor(i->valgrind_log, NULL);

			output = grab_file(i, i->valgrind_log, NULL);
			/* No valgrind errors? */
			if (!output || output[0] == '\0') {
				err = NULL;
				i->leak_info = NULL;
			} else {
				i->leak_info = analyze_output(output, &err);
			}
			if (err) {
				score_file_error(score, i, 0, "%s", err);
				score->pass = false;
			} else
				score->score++;
		}
	}
}

static void do_leakcheck_vg(struct manifest *m,
			    bool keep,
			    unsigned int *timeleft,
			    struct score *score)
{
	struct ccan_file *i;
	struct list_head *list;
	char **options;
	bool leaks = false;

	foreach_ptr(list, &m->run_tests, &m->api_tests) {
		list_for_each(list, i, list) {
			options = per_file_options(&tests_pass_valgrind_noleaks,
						   i);
			if (options[0]) {
				if (streq(options[0], "FAIL")) {
					leaks = true;
					continue;
				}
				errx(1, "Unknown leakcheck options '%s'",
				     options[0]);
			}

			if (i->leak_info) {
				score_file_error(score, i, 0, "%s",
						 i->leak_info);
				leaks = true;
			}
		}
	}

	/* FIXME: We don't fail for this, since many tests leak. */ 
	score->pass = true;
	if (!leaks) {
		score->score = 1;
	}
}

/* Gcc's warn_unused_result is fascist bullshit. */
#define doesnt_matter()

static void run_under_debugger_vg(struct manifest *m, struct score *score)
{
	struct file_error *first;
	char *command;

	/* Don't ask anything if they suppressed tests. */
	if (score->pass)
		return;

	if (!ask("Should I run the first failing test under the debugger?"))
		return;

	first = list_top(&score->per_file_errors, struct file_error, list);
	command = talloc_asprintf(m, "valgrind --leak-check=full --db-attach=yes%s %s",
				  concat(score,
					 per_file_options(&tests_pass_valgrind,
							  first->file)),
				  first->file->compiled);
	if (system(command))
		doesnt_matter();
}

struct ccanlint tests_pass_valgrind = {
	.key = "tests_pass_valgrind",
	.name = "Module's run and api tests succeed under valgrind",
	.can_run = can_run_vg,
	.check = do_run_tests_vg,
	.handle = run_under_debugger_vg,
	.takes_options = true,
	.needs = "tests_pass"
};

struct ccanlint tests_pass_valgrind_noleaks = {
	.key = "tests_pass_valgrind_noleaks",
	.name = "Module's run and api tests have no memory leaks",
	.check = do_leakcheck_vg,
	.takes_options = true,
	.needs = "tests_pass_valgrind"
};

