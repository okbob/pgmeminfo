# $PostgreSQL: pgsql/contrib/pgmeminfo/Makefile

MODULE_big = pgmeminfo
OBJS = $(patsubst %.c,%.o,$(wildcard src/*.c))
DATA = pgmeminfo--1.0.sql
EXTENSION = pgmeminfo

ifndef MAJORVERSION
MAJORVERSION := $(basename $(VERSION))
endif

ifdef NO_PGXS
subdir = contrib/pgmeminfo
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
else
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
endif
