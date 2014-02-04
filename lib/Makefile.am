### Makefile.am
include $(top_builddir)/version.mk

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_LDFLAGS =

headerdir = $(includedir)/dateutils

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

header_HEADERS =
header_HEADERS += date-core.h
header_HEADERS += date-core-strpf.h
header_HEADERS += time-core.h
header_HEADERS += time-core-strpf.h
header_HEADERS += dt-core.h
header_HEADERS += dt-core-strpf.h
header_HEADERS += strops.h
header_HEADERS += token.h
header_HEADERS += tzraw.h
header_HEADERS += leaps.h
header_HEADERS += boops.h
header_HEADERS += nifty.h
header_HEADERS += generics.h
header_HEADERS += dt-core-tz-glue.h
header_HEADERS += leapseconds.def

lib_LIBRARIES += libdut.a
libdut_a_CPPFLAGS = $(AM_CPPFLAGS)
libdut_a_CPPFLAGS += -DDECLF=extern
libdut_a_CPPFLAGS += -I$(builddir)
libdut_a_SOURCES =
libdut_a_SOURCES += version.c version.h version.c.in
libdut_a_SOURCES += date-core.h
libdut_a_SOURCES += time-core.h
libdut_a_SOURCES += dt-core.h dt-core.c
libdut_a_SOURCES += strops.h strops.c
libdut_a_SOURCES += token.h token.c
libdut_a_SOURCES += tzraw.h tzraw.c
libdut_a_SOURCES += tzmap.h tzmap.c
libdut_a_SOURCES += leaps.h leaps.c
libdut_a_SOURCES += boops.h
libdut_a_SOURCES += generics.h
libdut_a_SOURCES += dt-core-tz-glue.h dt-core-tz-glue.c
libdut_a_SOURCES += leapseconds.def leapseconds.h
EXTRA_libdut_a_SOURCES =
EXTRA_libdut_a_SOURCES += dt-core-strpf.c
EXTRA_libdut_a_SOURCES += dt-core-tz-glue.c
EXTRA_libdut_a_SOURCES += date-core.c
EXTRA_libdut_a_SOURCES += date-core-strpf.c
EXTRA_libdut_a_SOURCES += time-core.c
EXTRA_libdut_a_SOURCES += time-core-strpf.c
EXTRA_libdut_a_SOURCES += yd.c
EXTRA_libdut_a_SOURCES += ymd.c
EXTRA_libdut_a_SOURCES += ymcw.c
EXTRA_libdut_a_SOURCES += ywd.c
EXTRA_libdut_a_SOURCES += bizda.c
EXTRA_libdut_a_SOURCES += daisy.c
EXTRA_libdut_a_SOURCES += fmt-special.gperf
EXTRA_libdut_a_SOURCES += leapseconds
BUILT_SOURCES += version.c
BUILT_SOURCES += fmt-special.c
BUILT_SOURCES += leapseconds.def

if BUILD_LTRCC
noinst_PROGRAMS += ltrcc
ltrcc_CPPFLAGS = -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D_BSD_SOURCE
ltrcc_CPPFLAGS += -DDECLF=extern
ltrcc_CPPFLAGS += -DSKIP_LEAP_ARITH
ltrcc_SOURCES = ltrcc.c
ltrcc_SOURCES += dt-core.c dt-core.h
EXTRA_ltrcc_SOURCES =
EXTRA_ltrcc_SOURCES += fmt-special.gperf
endif  ## BUILD_LTRCC

noinst_PROGRAMS += tzmap
tzmap_SOURCES = tzmap.c tzmap.h tzmap.yuck
tzmap_CPPFLAGS = -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D_BSD_SOURCE
tzmap_CPPFLAGS += -DSTANDALONE
BUILT_SOURCES += tzmap.yucc

## some tzmaps we'd like to support
tzminfo_FILES =
tzminfo_FILES += iata.tzminfo
tzminfo_FILES += icao.tzminfo
EXTRA_DIST += $(tzminfo_FILES)

if HAVE_CURL
tzmap_FILES =
tzmap_FILES += iata.tzmap
tzmap_FILES += icao.tzmap
BUILT_SOURCES += $(tzmap_FILES)
DISTCLEANFILES += $(tzmap_FILES)

tzmcc_FILES =
tzmcc_FILES += iata.tzmcc
tzmcc_FILES += icao.tzmcc
pkgdata_DATA += $(tzmcc_FILES)
CLEANFILES += $(tzmcc_FILES)
DISTCLEANFILES += $(tzmcc_FILES)
endif  HAVE_CURL


AM_V_LTRCC = $(am__v_LTRCC_$(V))
am__v_LTRCC_ = $(am__v_LTRCC_$(AM_DEFAULT_VERBOSITY))
am__v_LTRCC_0 = @echo "  LTRCC   " $@;

leapseconds.def: leapseconds
	$(MAKE) $(AM_MAKEFLAGS) ltrcc$(EXEEXT)
	$(AM_V_LTRCC)$(builddir)/ltrcc$(EXEEXT) -C $< > $@ || rm -f $@

version.c: version.c.in $(top_builddir)/version.mk
	sed -e 's,[@]VERSION[@],$(VERSION),g' <$(srcdir)/$@.in >$@

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

## tzmap rules
SUFFIXES += .tzminfo
SUFFIXES += .tzmap
SUFFIXES += .tzmcc

.tzminfo.tzmap:
	curl -qgskO "https://raw.github.com/hroptatyr/dateutils/tzmaps/$@"

.tzmap.tzmcc:
	$(AM_V_GEN) $(builddir)/tzmap cc -o $@ $<

# 
# Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<
