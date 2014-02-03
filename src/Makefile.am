### Makefile.am
include $(top_builddir)/version.mk

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_LDFLAGS =

M4 = m4

bin_PROGRAMS =
noinst_PROGRAMS =
noinst_HEADERS =
noinst_LIBRARIES =
BUILT_SOURCES =
CLEANFILES =
GGO_HELPERS =
EXTRA_DIST = $(BUILT_SOURCES) $(GGO_HELPERS)
SUFFIXES =

DT_INCLUDES = -I$(top_srcdir)/lib -I$(top_builddir)/lib

noinst_LIBRARIES += libdutio.a
libdutio_a_SOURCES =
libdutio_a_SOURCES += dut-io.c dut-io.h
libdutio_a_SOURCES += dexpr.h
EXTRA_DIST += strpdt-special.gperf
BUILT_SOURCES += strpdt-special.c

bin_PROGRAMS += dseq
dseq_SOURCES = dseq.c dseq.yuck
dseq_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dseq_LDFLAGS = $(AM_LDFLAGS)
dseq_LDADD = $(top_builddir)/lib/libdut.a
BUILT_SOURCES += dseq.yucc

if HAVE_STRPTIME
bin_PROGRAMS += strptime
strptime_SOURCES = strptime.c strptime.yuck
strptime_SOURCES += prchunk.c prchunk.h
strptime_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
strptime_LDFLAGS = $(AM_LDFLAGS)
strptime_LDADD = $(top_builddir)/lib/libdut.a
endif
BUILT_SOURCES += strptime.yucc

bin_PROGRAMS += dconv
dconv_SOURCES = dconv.c dconv.yuck
dconv_SOURCES += prchunk.c prchunk.h
dconv_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dconv_LDFLAGS = $(AM_LDFLAGS)
dconv_LDADD = $(top_builddir)/lib/libdut.a
BUILT_SOURCES += dconv.yucc

bin_PROGRAMS += dtest
dtest_SOURCES = dtest.c dtest.yuck
dtest_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dtest_LDFLAGS = $(AM_LDFLAGS)
dtest_LDADD = $(top_builddir)/lib/libdut.a
BUILT_SOURCES += dtest.yucc

bin_PROGRAMS += dadd
dadd_SOURCES = dadd.c dadd.yuck
dadd_SOURCES += prchunk.c prchunk.h
dadd_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dadd_LDFLAGS = $(AM_LDFLAGS)
dadd_LDADD = $(top_builddir)/lib/libdut.a
BUILT_SOURCES += dadd.yucc

bin_PROGRAMS += ddiff
ddiff_SOURCES = ddiff.c ddiff.yuck
ddiff_SOURCES += prchunk.c prchunk.h
ddiff_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
ddiff_LDFLAGS = $(AM_LDFLAGS)
ddiff_LDADD = $(top_builddir)/lib/libdut.a
BUILT_SOURCES += ddiff.yucc

bin_PROGRAMS += dgrep
dgrep_SOURCES = dgrep.c dgrep.yuck
dgrep_SOURCES += prchunk.c prchunk.h
dgrep_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dgrep_LDFLAGS = $(AM_LDFLAGS)
dgrep_LDADD = $(top_builddir)/lib/libdut.a
BUILT_SOURCES += dgrep.yucc

bin_PROGRAMS += dround
dround_SOURCES = dround.c dround.yuck
dround_SOURCES += prchunk.c prchunk.h
dround_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dround_LDFLAGS = $(AM_LDFLAGS)
dround_LDADD = $(top_builddir)/lib/libdut.a
BUILT_SOURCES += dround.yucc

bin_PROGRAMS += dzone
dzone_SOURCES = dzone.c dzone.yuck
dzone_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dzone_LDFLAGS = $(AM_LDFLAGS)
dzone_LDADD = $(top_builddir)/lib/libdut.a
BUILT_SOURCES += dzone.yucc

if BUILD_DEXPR
noinst_PROGRAMS += dexpr
dexpr_SOURCES = dexpr.c
dexpr_SOURCES += dexpr-scanner.l
dexpr_SOURCES += dexpr-parser.y
dexpr_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES) -DSTANDALONE
dexpr_LDFLAGS = $(AM_LDFLAGS)
dexpr_LDADD = $(top_builddir)/lib/libdut.a
endif  ## BUILD_DEXPR
BUILT_SOURCES += dexpr-scanner.c dexpr-parser.c dexpr-parser.h


## yuck rule
SUFFIXES += .yuck
SUFFIXES += .yucc
.yuck.yucc:
	$(AM_V_GEN) PATH=$(top_builddir)/build-aux:"$${PATH}" \
		yuck$(EXEEXT) gen -o $@ $<

## gperf rule
SUFFIXES += .gperf
.gperf.c:
	$(AM_V_GEN) $(GPERF) -L ANSI-C $< --output-file $@


## just for parallel builds
dexpr-parser.h: dexpr-parser.c

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
