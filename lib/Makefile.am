### Makefile.am

AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_LDFLAGS =

headerdir = $(includedir)/dateutils

noinst_PROGRAMS =
noinst_HEADERS =
noinst_LIBRARIES =
lib_LIBRARIES =
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST =

header_HEADERS =
header_HEADERS += date-core.h date-core.c
header_HEADERS += date-core-strpf.h date-core-strpf.c
header_HEADERS += ymd.c
header_HEADERS += ymcw.c
header_HEADERS += ywd.c
header_HEADERS += time-core.h time-core.c
header_HEADERS += time-core-strpf.h time-core-strpf.c
header_HEADERS += dt-core.h dt-core.c
header_HEADERS += dt-core-strpf.h dt-core-strpf.c
header_HEADERS += strops.h strops.c
header_HEADERS += token.h token.c
header_HEADERS += tzraw.h tzraw.c
header_HEADERS += leaps.h leaps.c
header_HEADERS += boobs.h
header_HEADERS += nifty.h
header_HEADERS += generics.h
header_HEADERS += dt-core-tz-glue.h dt-core-tz-glue.c
header_HEADERS += leapseconds.def

lib_LIBRARIES += libdut.a
libdut_a_CPPFLAGS = $(AM_CPPFLAGS)
libdut_a_CPPFLAGS += -DDECLF=extern
libdut_a_CPPFLAGS += -I$(builddir)
libdut_a_SOURCES =
libdut_a_SOURCES += date-core.h
libdut_a_SOURCES += time-core.h
libdut_a_SOURCES += dt-core.h dt-core.c
libdut_a_SOURCES += strops.h strops.c
libdut_a_SOURCES += token.h token.c
libdut_a_SOURCES += tzraw.h tzraw.c
libdut_a_SOURCES += leaps.h leaps.c
libdut_a_SOURCES += boobs.h
libdut_a_SOURCES += generics.h
libdut_a_SOURCES += dt-core-tz-glue.h dt-core-tz-glue.c
libdut_a_SOURCES += leapseconds.def
EXTRA_libdut_a_SOURCES =
EXTRA_libdut_a_SOURCES += date-core.c
EXTRA_libdut_a_SOURCES += time-core.c
EXTRA_libdut_a_SOURCES += ymd.c
EXTRA_libdut_a_SOURCES += ymcw.c
EXTRA_libdut_a_SOURCES += ywd.c
EXTRA_libdut_a_SOURCES += fmt-special.gperf
if HAVE_GPERF
BUILT_SOURCES += fmt-special.c
endif  HAVE_GPERF

if BUILD_LTRCC
noinst_PROGRAMS += ltrcc
ltrcc_CPPFLAGS = -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D_BSD_SOURCE
ltrcc_CPPFLAGS += -DDECLF=extern
ltrcc_CPPFLAGS += -DSKIP_LEAP_ARITH
ltrcc_SOURCES = ltrcc.c
ltrcc_SOURCES += dt-core.c dt-core.h
endif  ## BUILD_LTRCC

EXTRA_DIST += leapseconds
BUILT_SOURCES += leapseconds.def
CLEANFILES += $(BUILT_SOURCES)

AM_V_LTRCC = $(am__v_LTRCC_$(V))
am__v_LTRCC_ = $(am__v_LTRCC_$(AM_DEFAULT_VERBOSITY))
am__v_LTRCC_0 = @echo "  LTRCC   " $@;

leapseconds.def: leapseconds
	$(MAKE) $(AM_MAKEFLAGS) ltrcc$(EXEEXT)
	$(AM_V_LTRCC)$(builddir)/ltrcc$(EXEEXT) -C $< > $@ || rm -f $@

## gperf rule
%.c: %.gperf
	$(AM_V_GEN) $(GPERF) $< --output-file $@

# 
# Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<
