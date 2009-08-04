#define _XOPEN_SOURCE 500
#include "tdb/tdb.h"
#include "tdb/io.c"
#include "tdb/tdb.c"
#include "tdb/lock.c"
#include "tdb/freelist.c"
#include "tdb/traverse.c"
#include "tdb/transaction.c"
#include "tdb/error.c"
#include "tdb/open.c"
#include "tap/tap.h"
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>
#include "external-transaction.h"

static int agent;

static bool correct_key(TDB_DATA key)
{
	return key.dsize == strlen("hi")
		&& memcmp(key.dptr, "hi", key.dsize) == 0;
}

static bool correct_data(TDB_DATA data)
{
	return data.dsize == strlen("world")
		&& memcmp(data.dptr, "world", data.dsize) == 0;
}

static int traverse2(struct tdb_context *tdb, TDB_DATA key, TDB_DATA data,
		     void *p)
{
	ok1(correct_key(key));
	ok1(correct_data(data));
	return 0;
}

static int traverse1(struct tdb_context *tdb, TDB_DATA key, TDB_DATA data,
		     void *p)
{
	ok1(correct_key(key));
	ok1(correct_data(data));
	ok1(!external_agent_transaction(agent, tdb_name(tdb)));
	tdb_traverse(tdb, traverse2, NULL);

	/* That should *not* release the transaction lock! */
	ok1(!external_agent_transaction(agent, tdb_name(tdb)));
	return 0;
}

int main(int argc, char *argv[])
{
	struct tdb_context *tdb;
	TDB_DATA key, data;

	plan_tests(15);
	agent = prepare_external_agent();
	if (agent < 0)
		err(1, "preparing agent");

	tdb = tdb_open("/tmp/test3.tdb", 1024, TDB_CLEAR_IF_FIRST,
		       O_CREAT|O_TRUNC|O_RDWR, 0600);
	ok1(tdb);

	ok1(external_agent_transaction(agent, tdb_name(tdb)));

	key.dsize = strlen("hi");
	key.dptr = (void *)"hi";
	data.dptr = (void *)"world";
	data.dsize = strlen("world");

	ok1(tdb_store(tdb, key, data, TDB_INSERT) == 0);
	tdb_traverse(tdb, traverse1, NULL);
	tdb_traverse_read(tdb, traverse1, NULL);
	tdb_close(tdb);

	return exit_status();
}