/*-------------------------------------------------------------------------
 *
 * pgmeminfo.c
 *
 *
 * by Pavel Stehule 2024-2024
 *
 *-------------------------------------------------------------------------
 *
 */
#include "postgres.h"

#include "fmgr.h"
#include "funcapi.h"

#include "access/tupdesc.h"
#include "access/htup.h"
#include "catalog/pg_type.h"

#include <malloc.h>

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pgmeminfo);

Datum
pgmeminfo(PG_FUNCTION_ARGS)
{

	Datum		values[10];
	bool		isnull[10];
	HeapTuple	tuple;
	TupleDesc	tupdesc;

#ifdef __GNU_LIBRARY__

/*
 * This test doesn't work well. Looks so HAVE_MALLINFO2 is undefined
 * although mallinfo2 function exists, and mallinfo is deprecated.
 * Maybe autoconf issue.
 */
#if (__GLIBC__ == 2 &&  __GLIBC_MINOR__ >= 33) || __GLIBC__ > 2

	struct mallinfo2 mi;

	mi = mallinfo2();

#else

	struct mallinfo mi;

	mi = mallinfo();

#endif

#else

	elog(ERROR, "this library requires __GNU_LIBRARY__");

#endif

	/*
	 * This record type had better match the output parameters declared for me
	 * in pg_proc.h.
	 */
	tupdesc = CreateTemplateTupleDesc(10);
	TupleDescInitEntry(tupdesc, (AttrNumber) 1,
					   "arena", INT8OID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 2,
					   "ordblks", INT8OID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 3,
					   "smblks", INT8OID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 4,
					   "hblks", INT8OID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 5,
					   "hblkd", INT8OID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 6,
					   "usmblks", INT8OID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 7,
					   "fsmblks", INT8OID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 8,
					   "uordblks", INT8OID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 9,
					   "fordblks", INT8OID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 10,
					   "keepcost", INT8OID, -1, 0);

	BlessTupleDesc(tupdesc);

	memset(isnull, false, sizeof(isnull));

	values[0] = Int64GetDatum((int64) mi.arena);
	values[1] = Int64GetDatum((int64) mi.ordblks);
	values[2] = Int64GetDatum((int64) mi.smblks);
	values[3] = Int64GetDatum((int64) mi.hblks);
	values[4] = Int64GetDatum((int64) mi.hblkhd);
	values[5] = Int64GetDatum((int64) mi.usmblks);
	values[6] = Int64GetDatum((int64) mi.fsmblks);
	values[7] = Int64GetDatum((int64) mi.uordblks);
	values[8] = Int64GetDatum((int64) mi.fordblks);
	values[9] = Int64GetDatum((int64) mi.keepcost);

	tuple = heap_form_tuple(tupdesc, values, isnull);

	PG_RETURN_DATUM(HeapTupleGetDatum(tuple));
}
