### Makefile.am

AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_LDFLAGS =
EXTRA_DIST =

if BUILD_MATCLI
moddir = $(HOME)/.matlab/$(shell basename "$(MATLABROOT)")/toolbox/dateutils

mod_LTLIBRARIES = tzconv.la
mod_DATA = tzconv.m
tzconv_la_LDFLAGS = -module -shrext ".mexa64"
tzconv_la_LDFLAGS += $(XCCLDFLAGS)
tzconv_la_CPPFLAGS = $(AM_CPPFLAGS) -DMATLAB_MEX_FILE
tzconv_la_CPPFLAGS += -I$(top_srcdir)/lib
tzconv_la_CPPFLAGS += -I$(top_builddir)/lib
tzconv_la_CPPFLAGS += -I$(MATLABROOT)/extern/include
tzconv_la_CPPFLAGS += -DDECLF=extern -DDEFUN=
tzconv_la_SOURCES = tzconv.c
tzconv_la_SOURCES += $(top_srcdir)/lib/tzraw.c $(top_srcdir)/lib/tzraw.h
tzconv_la_SOURCES += $(top_srcdir)/lib/leaps.c $(top_srcdir)/lib/leaps.h
EXTRA_DIST += $(mod_DATA)
endif  BUILD_MATCLI


# 
# Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<
