### Makefile.am

AM_CPPFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=201001L -D__USE_XOPEN2K8
AM_LDFLAGS =

M4 = m4

bin_PROGRAMS =
noinst_PROGRAMS =
BUILT_SOURCES =
CLEANFILES =
GGO_HELPERS =
EXTRA_DIST = $(BUILT_SOURCES) $(GGO_HELPERS)

GGO_HELPERS += __gengetopt-mularg.h __gengetopt-mularg.sed
GGO_HELPERS += common.ggo.in

DT_INCLUDES = -I$(top_srcdir)/lib
noinst_HEADERS = date-io.h

bin_PROGRAMS += dseq
dseq_SOURCES = dseq.c dseq-clo.ggo
dseq_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dseq_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dseq-clo.c dseq-clo.h

bin_PROGRAMS += strptime
strptime_SOURCES = strptime.c strptime-clo.ggo
strptime_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
strptime_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += strptime-clo.c strptime-clo.h

bin_PROGRAMS += dcal
dcal_SOURCES = dcal.c dcal-clo.ggo
dcal_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dcal_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dcal-clo.c dcal-clo.h

bin_PROGRAMS += dtest
dtest_SOURCES = dtest.c dtest-clo.ggo
dtest_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dtest_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dtest-clo.c dtest-clo.h

bin_PROGRAMS += dadd
dadd_SOURCES = dadd.c dadd-clo.ggo
dadd_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dadd_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dadd-clo.c dadd-clo.h

bin_PROGRAMS += ddiff
ddiff_SOURCES = ddiff.c ddiff-clo.ggo
ddiff_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
ddiff_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += ddiff-clo.c ddiff-clo.h


## ggo rule
%.c %.h: %.ggo $(GGO_HELPERS)
	$(M4) -I $(srcdir) common.ggo.in $< | gengetopt -l -F $*
## patch the multiple arg thingie dingie
	sed -i -f $(srcdir)/__gengetopt-mularg.sed -- $*.c

# 
# Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<
