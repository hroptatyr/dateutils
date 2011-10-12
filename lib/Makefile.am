### Makefile.am

AM_CPPFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=201001L -D__USE_XOPEN2K8
AM_LDFLAGS =

headerdir = $(includedir)/dateutils

noinst_PROGRAMS =
noinst_HEADERS =
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST = $(BUILT_SOURCES)

header_HEADERS =
header_HEADERS += date-core.h date-core.c
header_HEADERS += time-core.h time-core.c
header_HEADERS += strops.h strops.c

noinst_PROGRAMS += testlib
testlib_CPPFLAGS = $(AM_CPPFLAGS)
testlib_LDFLAGS = $(AM_LDFLAGS)


# 
# Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<
