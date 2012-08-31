### Makefile.am

AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_LDFLAGS =

M4 = m4

bin_PROGRAMS =
noinst_PROGRAMS =
noinst_HEADERS =
BUILT_SOURCES =
CLEANFILES =
GGO_HELPERS =
EXTRA_DIST = $(BUILT_SOURCES) $(GGO_HELPERS)

GGO_HELPERS += __gengetopt-mularg.h __gengetopt-mularg.sed
GGO_HELPERS += common.ggo.in

DT_INCLUDES = -I$(top_srcdir)/lib -I$(top_builddir)/lib
noinst_HEADERS += dt-io.h
noinst_HEADERS += dexpr.h
EXTRA_DIST += strpdt-special.gperf
if HAVE_GPERF
BUILT_SOURCES += strpdt-special.c
endif  HAVE_GPERF

bin_PROGRAMS += dseq
dseq_SOURCES = dseq.c dseq-clo.ggo
dseq_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dseq_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dseq-clo.c dseq-clo.h

if HAVE_STRPTIME
bin_PROGRAMS += strptime
strptime_SOURCES = strptime.c strptime-clo.ggo
strptime_SOURCES += prchunk.c prchunk.h
strptime_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
strptime_LDFLAGS = $(AM_LDFLAGS)
endif
BUILT_SOURCES += strptime-clo.c strptime-clo.h

bin_PROGRAMS += dconv
dconv_SOURCES = dconv.c dconv-clo.ggo
dconv_SOURCES += prchunk.c prchunk.h
dconv_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dconv_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dconv-clo.c dconv-clo.h

bin_PROGRAMS += dtest
dtest_SOURCES = dtest.c dtest-clo.ggo
dtest_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dtest_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dtest-clo.c dtest-clo.h

bin_PROGRAMS += dadd
dadd_SOURCES = dadd.c dadd-clo.ggo
dadd_SOURCES += prchunk.c prchunk.h
dadd_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dadd_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dadd-clo.c dadd-clo.h

bin_PROGRAMS += ddiff
ddiff_SOURCES = ddiff.c ddiff-clo.ggo
ddiff_SOURCES += prchunk.c prchunk.h
ddiff_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
ddiff_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += ddiff-clo.c ddiff-clo.h

bin_PROGRAMS += dgrep
dgrep_SOURCES = dgrep.c dgrep-clo.ggo
dgrep_SOURCES += prchunk.c prchunk.h
dgrep_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dgrep_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dgrep-clo.c dgrep-clo.h

bin_PROGRAMS += dround
dround_SOURCES = dround.c dround-clo.ggo
dround_SOURCES += prchunk.c prchunk.h
dround_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dround_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dround-clo.c dround-clo.h

if BUILD_DEXPR
noinst_PROGRAMS += dexpr
dexpr_SOURCES = dexpr.c
dexpr_SOURCES += dexpr-scanner.l
dexpr_SOURCES += dexpr-parser.y
dexpr_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES) -DSTANDALONE
dexpr_LDFLAGS = $(AM_LDFLAGS)
endif  ## BUILD_DEXPR
BUILT_SOURCES += dexpr-scanner.c dexpr-parser.c dexpr-parser.h


## ggo rule
%.c %.h: %.ggo $(GGO_HELPERS)
	$(AM_V_GEN)$(M4) -I $(srcdir) common.ggo.in $< | gengetopt -l -F $*
## patch the multiple arg thingie dingie
	@sed -f $(srcdir)/__gengetopt-mularg.sed -- $*.c > $*.c.sed && \
		mv $*.c.sed $*.c

## gperf rule
%.c: %.gperf
	$(AM_V_GEN) $(GPERF) $< --output-file $@


## for dist or dist check we need binaries, so help2man can work
dist-hook: $(bin_PROGRAMS)

# 
# Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<
