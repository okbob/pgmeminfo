-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pgmeminfo" to load this file. \quit

/*
 * Total non-mmapped bytes (arena):       %ld\n", mi.arena);
 * # of free chunks (ordblks):            %ld\n", mi.ordblks);
 * # of free fastbin blocks (smblks):     %ld\n", mi.smblks);
 * # of mapped regions (hblks):           %ld\n", mi.hblks);
 * Bytes in mapped regions (hblkhd):      %ld\n", mi.hblkhd);
 * Max. total allocated space (usmblks):  %ld\n", mi.usmblks);
 * Free bytes held in fastbins (fsmblks): %ld\n", mi.fsmblks);
 * Total allocated space (uordblks):      %ld\n", mi.uordblks);
 * Total free space (fordblks):           %ld\n", mi.fordblks);
 * Topmost releasable block (keepcost):   %ld\n", mi.keepcost);
 */
CREATE FUNCTION pgmeminfo(OUT arena int8, OUT ordblks int8, OUT smblks int8,
                          OUT hblks int8, OUT hblkhd int8, OUT usmblks int8,
                          OUT fsmblks int8, OUT uordblks int8, OUT fordblks int8,
                          OUT keepcost int8)
RETURNS RECORD
AS 'MODULE_PATHNAME','pgmeminfo'
LANGUAGE C VOLATILE;


CREATE FUNCTION pgmeminfo_contexts(IN deep int DEFAULT 0,
                                   IN accum_mode text DEFAULT 'all',
                                   OUT name text,
                                   OUT ident text,
                                   OUT parent text,
                                   OUT level int,
                                   OUT total_bytes int8,
                                   OUT total_nblocks int8,
                                   OUT free_bytes int8,
                                   OUT used_bytes int8)
RETURNS SETOF RECORD
AS 'MODULE_PATHNAME','pgmeminfo_contexts'
LANGUAGE C VOLATILE STRICT;
