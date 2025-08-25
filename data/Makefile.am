### Makefile.am
include $(top_builddir)/version.mk

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_LDFLAGS =

bin_PROGRAMS =
noinst_PROGRAMS =
noinst_HEADERS =
noinst_LIBRARIES =
pkgdata_DATA =
BUILT_SOURCES =
CLEANFILES =
GGO_HELPERS =
EXTRA_DIST = $(BUILT_SOURCES) $(pkgdata_DATA)
SUFFIXES =

pkgdata_DATA += locale
pkgdata_DATA += ummulqura.tab
EXTRA_DIST += snarf-ummulqura.R

ummulqura.tab: snarf-ummulqura.R
	$< --as.c -o $@.t $(shell seq 1318 1451) \
	&& mv $@.t $@
