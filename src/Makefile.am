### Makefile.am

AM_CPPFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=201001L -D__USE_XOPEN2K8
AM_LDFLAGS =

bin_PROGRAMS =
noinst_PROGRAMS =
noinst_HEADERS =
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST = $(BUILT_SOURCES)
EXTRA_DIST += __gengetopt-mularg.h __gengetopt-mularg.sed

bin_PROGRAMS += dseq
dseq_SOURCES = dseq.c dseq-clo.ggo
dseq_CPPFLAGS = $(AM_CPPFLAGS)
dseq_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dseq-clo.c dseq-clo.h

bin_PROGRAMS += strptime
strptime_SOURCES = strptime.c strptime-clo.ggo
strptime_CPPFLAGS = $(AM_CPPFLAGS)
strptime_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += strptime-clo.c strptime-clo.h

bin_PROGRAMS += dcal
dcal_SOURCES = dcal.c dcal-clo.ggop
dcal_CPPFLAGS = $(AM_CPPFLAGS)
dcal_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dcal-clo.c dcal-clo.h

bin_PROGRAMS += dtest
dtest_SOURCES = dtest.c dtest-clo.ggop
dtest_CPPFLAGS = $(AM_CPPFLAGS)
dtest_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dtest-clo.c dtest-clo.h


## ggo rule
%.c %.h: %.ggo $(srcdir)/__gengetopt-mularg.sed
	gengetopt -l -i $< -F $*
## patch the multiple arg thingie dingie
	sed -i -f $(srcdir)/__gengetopt-mularg.sed -- $*.c

## ggo rule without multiple patch
%.c %.h: %.ggop
	gengetopt -l -i $< -F $*

# 
# Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<
