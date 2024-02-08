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
#include "miscadmin.h"

#include "access/tupdesc.h"
#include "access/htup.h"
#include "catalog/pg_type.h"

#include "mb/pg_wchar.h"
#include "utils/builtins.h"

#include <malloc.h>

#ifdef _MSC_VER

#define strcasecmp _stricmp
#define strncasecmp _strnicmp

#endif

#define MEMORY_CONTEXT_IDENT_DISPLAY_SIZE	1024


PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pgmeminfo);
PG_FUNCTION_INFO_V1(pgmeminfo_contexts);

typedef enum
{
	ACCUM_ALL,
	ACCUM_OFF,
	ACCUM_LEAF,
} AccumMode;

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

static void
PutMemoryContextStatsTupleStore(AccumMode acm, int deep,
								Tuplestorestate *tupstore, TupleDesc tupdesc,
								MemoryContext context,
								const char *parent, int level)
{
#define PGMEMINFO_CONTEXTS_COLS	8

	Datum			values[PGMEMINFO_CONTEXTS_COLS];
	bool			nulls[PGMEMINFO_CONTEXTS_COLS];
	MemoryContextCounters stat;
	MemoryContext child;
	const char	   *name;
	const char	   *ident;

	Assert(MemoryContextIsValid(context));

	name = context->name;
	ident = context->ident;

	memset(&stat, 0, sizeof(stat));
	if (acm == ACCUM_ALL || (acm == ACCUM_LEAF && level == deep))
		MemoryContextMemConsumed(context, &stat);
	else
		(*context->methods->stats) (context, NULL, (void *) &level, &stat, true);

	memset(values, 0, sizeof(values));
	memset(nulls, 0, sizeof(nulls));

	if (name)
		values[0] = CStringGetTextDatum(name);
	else
		nulls[0] = true;

	if (ident)
	{
		int			idlen = strlen(ident);
		char		clipped_ident[MEMORY_CONTEXT_IDENT_DISPLAY_SIZE];

		/*
		 * Some identifiers such as SQL query string can be very long,
		 * truncate oversize identifiers.
		 */
		if (idlen >= MEMORY_CONTEXT_IDENT_DISPLAY_SIZE)
			idlen = pg_mbcliplen(ident, idlen, MEMORY_CONTEXT_IDENT_DISPLAY_SIZE - 1);

		memcpy(clipped_ident, ident, idlen);
		clipped_ident[idlen] = '\0';
		values[1] = CStringGetTextDatum(clipped_ident);
	}
	else
		nulls[1] = true;

	if (parent)
		values[2] = CStringGetTextDatum(parent);
	else
		nulls[2] = true;

	values[3] = Int32GetDatum(level);

	values[4] = Int64GetDatum(stat.totalspace);
	values[5] = Int64GetDatum(stat.nblocks);
	values[6] = Int64GetDatum(stat.freespace);
	values[7] = Int64GetDatum(stat.totalspace - stat.freespace);

	tuplestore_putvalues(tupstore, tupdesc, values, nulls);

	if (deep == level)
		return;

	for (child = context->firstchild; child != NULL; child = child->nextchild)
	{
		PutMemoryContextStatsTupleStore(acm, deep,
										tupstore, tupdesc,
										child,
										name, level + 1);
	}
}

/*
 * CREATE FUNCTION pgmeminfo_contexts(IN deep int DEFAULT 0,
 *                                    IN accum_mode text DEFAULT "all",
 *                                    OUT name text,
 *                                    OUT ident text,
 *                                    OUT parent text,
 *                                    OUT level int,
 *                                    OUT total_bytes bigint,
 *                                    OUT total_nblocks int,
 *                                    OUT free_bytes bigint,
 *                                    OUT used_bytes)
 *
 */
Datum
pgmeminfo_contexts(PG_FUNCTION_ARGS)
{
	ReturnSetInfo	   *rsinfo;
	TupleDesc			tupdesc;
	Tuplestorestate	   *tupstore;
	MemoryContext		per_query_ctx;
	MemoryContext		oldcxt;
	int					deep;
	char				*accum_mode_txt;
	AccumMode			acm;

	rsinfo = (ReturnSetInfo *) fcinfo->resultinfo;

	/* check to see if caller supports us returning a tuplestore */
	if (rsinfo == NULL || !IsA(rsinfo, ReturnSetInfo))
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("set-valued function called in context that cannot accept a set")));

	if (!(rsinfo->allowedModes & SFRM_Materialize))
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("materialize mode required, but it is not allowed in this context")));

	/* Build a tuple descriptor for our result type */
	if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
		elog(ERROR, "return type must be a row type");

	deep = PG_GETARG_INT32(0);
	accum_mode_txt = text_to_cstring(PG_GETARG_TEXT_PP(1));

	if (strcasecmp(accum_mode_txt, "all") == 0)
		acm = ACCUM_ALL;
	else if (strcasecmp(accum_mode_txt, "off") == 0)
		acm = ACCUM_OFF;
	else if (strcasecmp(accum_mode_txt, "leaf") == 0)
		acm = ACCUM_LEAF;
	else
		elog(ERROR, "unsupported value for of accum_mode argument");


	per_query_ctx = rsinfo->econtext->ecxt_per_query_memory;
	oldcxt = MemoryContextSwitchTo(per_query_ctx);

	tupstore = tuplestore_begin_heap(true, false, work_mem);
	rsinfo->returnMode = SFRM_Materialize;
	rsinfo->setResult = tupstore;
	rsinfo->setDesc = tupdesc;

	MemoryContextSwitchTo(oldcxt);

	PutMemoryContextStatsTupleStore(acm, deep,
									tupstore, tupdesc,
									TopMemoryContext,
									NULL, 0);

	PG_RETURN_VOID();
}
