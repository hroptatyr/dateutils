### Makefile.am
include $(top_builddir)/version.mk

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE -D_DEFAULT_SOURCE
AM_LDFLAGS =

noinst_PROGRAMS =
noinst_HEADERS =
noinst_LIBRARIES =
pkgdata_DATA =
lib_LIBRARIES =
BUILT_SOURCES =
CLEANFILES =
DISTCLEANFILES =
EXTRA_DIST = $(BUILT_SOURCES)
SUFFIXES =

ANALYZE = clang
ANALYZE_FLAGS = "--analyze"
# Based on COMPILE target
ANALYZE_COMMAND = $(ANALYZE) \
	$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) \
	$(ANALYZE_FLAGS)

if BUILD_LTRCC
noinst_PROGRAMS += ltrcc
ltrcc_CPPFLAGS = -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D_BSD_SOURCE -D_DEFAULT_SOURCE
ltrcc_CPPFLAGS += -DDECLF=extern
ltrcc_CPPFLAGS += -DSKIP_LEAP_ARITH
ltrcc_SOURCES = ltrcc.c ltrcc.yuck

ltrcc_SOURCES += date-core.c date-core.h
ltrcc_SOURCES += time-core.c time-core.h
ltrcc_SOURCES += dt-locale.c dt-locale.h
ltrcc_SOURCES += token.c token.h
ltrcc_SOURCES += strops.c strops.h
endif  ## BUILD_LTRCC
BUILT_SOURCES += ltrcc.yucc

noinst_LIBRARIES += libdut.a
libdut_a_CPPFLAGS = $(AM_CPPFLAGS)
libdut_a_CPPFLAGS += -DDECLF=extern
libdut_a_CPPFLAGS += -DLIBDUT
libdut_a_CPPFLAGS += -I$(builddir)
libdut_a_CPPFLAGS += -DLOCALE_FILE='"$(pkgdatadir)/locale"'
libdut_a_SOURCES =
libdut_a_SOURCES += version.c version.h version.c.in
libdut_a_SOURCES += date-core.c date-core.h date-core-private.h
libdut_a_SOURCES += date-core-strpf.h
libdut_a_SOURCES += time-core.c time-core.h time-core-private.h
libdut_a_SOURCES += time-core-strpf.h
libdut_a_SOURCES += dt-core.c dt-core.h dt-core-private.h
libdut_a_SOURCES += dt-core-strpf.h
libdut_a_SOURCES += strops.h strops.c
libdut_a_SOURCES += token.h token.c
libdut_a_SOURCES += tzraw.h tzraw.c
libdut_a_SOURCES += tzmap.h tzmap.c
libdut_a_SOURCES += leaps.h leaps.c
libdut_a_SOURCES += dt-locale.h dt-locale.c
libdut_a_SOURCES += boops.h
libdut_a_SOURCES += nifty.h
libdut_a_SOURCES += gmtime.h
libdut_a_SOURCES += generics.h
libdut_a_SOURCES += dt-core-tz-glue.h dt-core-tz-glue.c
libdut_a_SOURCES += leap-seconds.def leap-seconds.h
EXTRA_libdut_a_SOURCES =
EXTRA_libdut_a_SOURCES += dt-core-strpf.c
EXTRA_libdut_a_SOURCES += dt-core-tz-glue.c
EXTRA_libdut_a_SOURCES += date-core-strpf.c
EXTRA_libdut_a_SOURCES += time-core-strpf.c
EXTRA_libdut_a_SOURCES += yd.c
EXTRA_libdut_a_SOURCES += ymd.c
EXTRA_libdut_a_SOURCES += ymcw.c
EXTRA_libdut_a_SOURCES += ywd.c
EXTRA_libdut_a_SOURCES += bizda.c
EXTRA_libdut_a_SOURCES += daisy.c
EXTRA_libdut_a_SOURCES += fmt-special.gperf
EXTRA_libdut_a_SOURCES += leap-seconds.list
EXTRA_libdut_a_SOURCES += ummulqura.c
BUILT_SOURCES += version.c
BUILT_SOURCES += fmt-special.c
BUILT_SOURCES += leap-seconds.def

noinst_PROGRAMS += tzmap
tzmap_SOURCES = tzmap.c tzmap.h tzmap.yuck
tzmap_CPPFLAGS = -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D_BSD_SOURCE -D_DEFAULT_SOURCE
tzmap_CPPFLAGS += -DSTANDALONE
BUILT_SOURCES += tzmap.yucc

noinst_PROGRAMS += tzraw
tzraw_SOURCES = tzraw.c tzraw.h
tzraw_SOURCES += leaps.h leaps.c
tzraw_CPPFLAGS = -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D_BSD_SOURCE -D_DEFAULT_SOURCE
tzraw_CPPFLAGS += -DSTANDALONE

## some tzmaps we'd like to support
tzminfo_FILES =
tzminfo_FILES += iata.tzminfo
tzminfo_FILES += icao.tzminfo
tzminfo_FILES += mic.tzminfo
EXTRA_DIST += $(tzminfo_FILES)

tzmap_FILES =
tzmcc_FILES =

if FETCH_TZMAPS
tzmap_FILES += iata.tzmap
tzmap_FILES += icao.tzmap
tzmap_FILES += mic.tzmap
BUILT_SOURCES += $(tzmap_FILES)
DISTCLEANFILES += $(tzmap_FILES)

tzmcc_FILES += iata.tzmcc
tzmcc_FILES += icao.tzmcc
tzmcc_FILES += mic.tzmcc
pkgdata_DATA += $(tzmcc_FILES)
CLEANFILES += $(tzmcc_FILES)
DISTCLEANFILES += $(tzmcc_FILES)
endif  FETCH_TZMAPS

if HAVE_TZMAPS
## same thing as with the fetching
tzmcc_FILES += iata.tzmcc
tzmcc_FILES += icao.tzmcc
tzmcc_FILES += mic.tzmcc
pkgdata_DATA += $(tzmcc_FILES)
CLEANFILES += $(tzmcc_FILES)
DISTCLEANFILES += $(tzmcc_FILES)
endif  HAVE_TZMAPS


AM_V_LTRCC = $(am__v_LTRCC_$(V))
am__v_LTRCC_ = $(am__v_LTRCC_$(AM_DEFAULT_VERBOSITY))
am__v_LTRCC_0 = @echo "  LTRCC   " $@;

AM_V_DL = $(am__v_DL_$(V))
am__v_DL_ = $(am__v_DL_$(AM_DEFAULT_VERBOSITY))
am__v_DL_0 = @echo "  D/L'ING " $@;

## leaps
SUFFIXES += .list
SUFFIXES += .def
.list.def:
	$(MAKE) $(AM_MAKEFLAGS) ltrcc$(EXEEXT)
	$(AM_V_LTRCC)$(builddir)/ltrcc$(EXEEXT) -C $< > $@ || rm -f $@

## version rules
version.c: version.c.in $(top_builddir)/.version
	$(AM_V_GEN) PATH="$(top_builddir)/build-aux:$${PATH}" \
		$(YUCK) scmver --ignore-noscm --force -o $@ \
			--use-reference --reference $(top_builddir)/.version \
			$(srcdir)/version.c.in

## yuck rule
SUFFIXES += .yuck
SUFFIXES += .yucc
.yuck.yucc:
	$(AM_V_GEN) PATH="$(top_builddir)/build-aux:$${PATH}" \
		$(YUCK) gen -o $@ $<

## gperf rule
SUFFIXES += .gperf
.gperf.c:
	$(AM_V_GEN) $(GPERF) -L ANSI-C $< --output-file $@

## tzmap rules
SUFFIXES += .tzminfo
SUFFIXES += .tzmap
SUFFIXES += .tzmcc

TZMAP_URL := https://raw.github.com/hroptatyr/dateutils/tzmaps
.tzminfo.tzmap:
	-$(AM_V_DL) curl -q -gskfLO -D "$*.hdr" "$(TZMAP_URL)/$@"

.tzmap.tzmcc:
	-$(AM_V_GEN) $(builddir)/tzmap cc -e -o $@ $<

## make sure installing the tzmaps isn't fatal either
.IGNORE: install-pkgdataDATA

## for dist or distcheck we need libdut.a
dist-hook: $(noinst_LIBRARIES)

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
