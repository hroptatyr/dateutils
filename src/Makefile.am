### Makefile.am

AM_CPPFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=201001L -D__USE_XOPEN2K8
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

DT_INCLUDES = -I$(top_srcdir)/lib
noinst_HEADERS += date-io.h
noinst_HEADERS += time-io.h
noinst_HEADERS += dt-io.h
noinst_HEADERS += dexpr.h

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

bin_PROGRAMS += tseq
tseq_SOURCES = tseq.c tseq-clo.ggo
tseq_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
tseq_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += tseq-clo.c tseq-clo.h

bin_PROGRAMS += tdiff
tdiff_SOURCES = tdiff.c tdiff-clo.ggo
tdiff_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
tdiff_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += tdiff-clo.c tdiff-clo.h

bin_PROGRAMS += tadd
tadd_SOURCES = tadd.c tadd-clo.ggo
tadd_SOURCES += prchunk.c prchunk.h
tadd_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
tadd_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += tadd-clo.c tadd-clo.h

bin_PROGRAMS += tgrep
tgrep_SOURCES = tgrep.c tgrep-clo.ggo
tgrep_SOURCES += prchunk.c prchunk.h
tgrep_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
tgrep_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += tgrep-clo.c tgrep-clo.h

bin_PROGRAMS += ttest
ttest_SOURCES = ttest.c ttest-clo.ggo
ttest_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
ttest_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += ttest-clo.c ttest-clo.h

bin_PROGRAMS += tconv
tconv_SOURCES = tconv.c tconv-clo.ggo
tconv_SOURCES += prchunk.c prchunk.h
tconv_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
tconv_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += tconv-clo.c tconv-clo.h

noinst_PROGRAMS += dexpr
dexpr_SOURCES = dexpr.c
dexpr_SOURCES += dexpr-scanner.l
dexpr_SOURCES += dexpr-parser.y
dexpr_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES) -DSTANDALONE
dexpr_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dexpr-scanner.c dexpr-parser.c dexpr-parser.h


bin_PROGRAMS += dtconv
dtconv_SOURCES = dtconv.c dtconv-clo.ggo
dtconv_SOURCES += prchunk.h prchunk.c
dtconv_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dtconv_LDFLAGS = $(AM_LDFLAGS)
BUILT_SOURCES += dtconv-clo.c dtconv-clo.h


## ggo rule
%.c %.h: %.ggo $(GGO_HELPERS)
	$(M4) -I $(srcdir) common.ggo.in $< | gengetopt -l -F $*
## patch the multiple arg thingie dingie
	sed -f $(srcdir)/__gengetopt-mularg.sed -- $*.c > $*.c.sed && \
		mv $*.c.sed $*.c

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
