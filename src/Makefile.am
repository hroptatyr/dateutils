### Makefile.am
include $(top_builddir)/version.mk

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE -D_DEFAULT_SOURCE
AM_CPPFLAGS += -DHAVE_VERSION_H
AM_LDFLAGS =
AM_YFLAGS = -d

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
DT_LIBS = $(top_builddir)/lib/libdut.a

ANALYZE = clang
ANALYZE_FLAGS = "--analyze"
# Based on COMPILE target
ANALYZE_COMMAND = $(ANALYZE) \
	$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(DT_INCLUDES) $(AM_CFLAGS) $(CFLAGS) \
	$(ANALYZE_FLAGS)

noinst_LIBRARIES += libdutio.a
libdutio_a_SOURCES =
libdutio_a_SOURCES += dt-io.c dt-io.h
libdutio_a_SOURCES += dt-io-zone.c dt-io-zone.h
libdutio_a_SOURCES += alist.c alist.h
libdutio_a_SOURCES += prchunk.c prchunk.h
libdutio_a_SOURCES += dexpr.h
libdutio_a_CPPFLAGS = $(AM_CPPFLAGS)
libdutio_a_CPPFLAGS += $(DT_INCLUDES)
libdutio_a_CPPFLAGS += -DTZMAP_DIR='"$(pkgdatadir)"'
EXTRA_DIST += strpdt-special.gperf
BUILT_SOURCES += strpdt-special.c

## all them binaries, alphabetical
bin_PROGRAMS += dadd
bin_PROGRAMS += dconv
bin_PROGRAMS += ddiff
bin_PROGRAMS += dgrep
bin_PROGRAMS += dround
bin_PROGRAMS += dseq
bin_PROGRAMS += dsort
bin_PROGRAMS += dtest
bin_PROGRAMS += dzone

if WITH_OLD_LINKS
CREATE_OLD_LINKS =  $(LN_S) date$$prog$(EXEEXT) d$$prog$(EXEEXT);
endif

if !WITH_OLD_NAMES
install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
		for prog in add conv diff grep round seq sort test zone; do \
			mv -f d$$prog$(EXEEXT) date$$prog$(EXEEXT) ; \
			$(CREATE_OLD_LINKS) \
		done

uninstall-hook:
	cd $(DESTDIR)$(bindir) && \
		for prog in add conv diff grep round seq sort test zone; do \
			$(RM) date$$prog$(EXEEXT) ; \
		done
endif  ## !WITH_OLD_NAMES

dseq_SOURCES = dseq.c dseq.yuck
dseq_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dseq_LDFLAGS = $(AM_LDFLAGS)
dseq_LDADD = libdutio.a
dseq_LDADD += $(DT_LIBS)
BUILT_SOURCES += dseq.yucc

if HAVE_STRPTIME
bin_PROGRAMS += strptime
strptime_SOURCES = strptime.c strptime.yuck
strptime_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
strptime_LDFLAGS = $(AM_LDFLAGS)
strptime_LDADD = libdutio.a
strptime_LDADD += $(DT_LIBS)
endif
BUILT_SOURCES += strptime.yucc

dconv_SOURCES = dconv.c dconv.yuck
dconv_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dconv_LDFLAGS = $(AM_LDFLAGS)
dconv_LDADD = libdutio.a
dconv_LDADD += $(DT_LIBS)
BUILT_SOURCES += dconv.yucc

dtest_SOURCES = dtest.c dtest.yuck
dtest_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dtest_LDFLAGS = $(AM_LDFLAGS)
dtest_LDADD = libdutio.a
dtest_LDADD += $(DT_LIBS)
BUILT_SOURCES += dtest.yucc

dadd_SOURCES = dadd.c dadd.yuck
dadd_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dadd_LDFLAGS = $(AM_LDFLAGS)
dadd_LDADD = libdutio.a
dadd_LDADD += $(DT_LIBS)
BUILT_SOURCES += dadd.yucc

ddiff_SOURCES = ddiff.c ddiff.yuck
ddiff_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
ddiff_LDFLAGS = $(AM_LDFLAGS)
ddiff_LDADD = libdutio.a
ddiff_LDADD += $(DT_LIBS)
BUILT_SOURCES += ddiff.yucc

dgrep_SOURCES = dgrep.c dgrep.yuck
dgrep_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dgrep_LDFLAGS = $(AM_LDFLAGS)
dgrep_LDADD = libdutio.a
dgrep_LDADD += $(DT_LIBS)
BUILT_SOURCES += dgrep.yucc

dround_SOURCES = dround.c dround.yuck
dround_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dround_LDFLAGS = $(AM_LDFLAGS)
dround_LDADD = libdutio.a
dround_LDADD += $(DT_LIBS)
BUILT_SOURCES += dround.yucc

dzone_SOURCES = dzone.c dzone.yuck
dzone_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dzone_LDFLAGS = $(AM_LDFLAGS)
dzone_LDADD = libdutio.a
dzone_LDADD += $(DT_LIBS)
BUILT_SOURCES += dzone.yucc

dsort_SOURCES = dsort.c dsort.yuck
dsort_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES)
dsort_LDFLAGS = $(AM_LDFLAGS)
dsort_LDADD = libdutio.a
dsort_LDADD += $(DT_LIBS)
BUILT_SOURCES += dsort.yucc

if BUILD_DEXPR
noinst_PROGRAMS += dexpr
dexpr_SOURCES = dexpr.c
dexpr_SOURCES += dexpr-scanner.l
dexpr_SOURCES += dexpr-parser.y
dexpr_CPPFLAGS = $(AM_CPPFLAGS) $(DT_INCLUDES) -DSTANDALONE
dexpr_LDFLAGS = $(AM_LDFLAGS)
dexpr_LDADD = libdutio.a
dexpr_LDADD += $(DT_LIBS)
endif  ## BUILD_DEXPR
BUILT_SOURCES += dexpr-scanner.c dexpr-parser.c dexpr-parser.h


## yuck rule
SUFFIXES += .yuck
SUFFIXES += .yucc
.yuck.yucc:
	$(AM_V_GEN) PATH=$(top_builddir)/build-aux:"$${PATH}" \
		$(YUCK) gen -o $@ $<

## gperf rule
SUFFIXES += .gperf
.gperf.c:
	$(AM_V_GEN) $(GPERF) -L ANSI-C $< --output-file $@


## just for parallel builds
dexpr-parser.h: dexpr-parser.c

# 
# Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<

## Run Clang static analyzer over sources.
anal: $(SOURCES)
	@list='$(SOURCES)'; \
	result=0; \
	for file in $$list; do \
		if echo $$file | grep '\.c$$' >/dev/null 2>&1; then \
			echo "Analyzing $$file"; \
			$(ANALYZE_COMMAND) $(srcdir)/$$file; \
			status=$$?; \
			if test $$status != 0; then \
				result=1; \
			fi; \
		fi; \
	done; \
	set -e; exit $$result
