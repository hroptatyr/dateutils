### Makefile.am

AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_LDFLAGS =
EXTRA_DIST =

MEX_LDFLAGS = -module -shared -avoid-version -shrext ".mexa64"

if BUILD_MATCLI
mexdir = $(HOME)/.matlab/$(shell basename "$(MATLABROOT)")/toolbox/dateutils
mex_LTLIBRARIES =
mex_DATA =

mex_LTLIBRARIES += tzconv.la
mex_DATA += tzconv.m
tzconv_la_LDFLAGS = $(AM_LDFLAGS)
tzconv_la_LDFLAGS += $(MEX_LDFLAGS)
tzconv_la_LDFLAGS += $(XCCLDFLAGS)
tzconv_la_CPPFLAGS = $(AM_CPPFLAGS) -DMATLAB_MEX_FILE
tzconv_la_CPPFLAGS += -I$(top_srcdir)/lib
tzconv_la_CPPFLAGS += -I$(top_builddir)/lib
tzconv_la_CPPFLAGS += $(matlab_CFLAGS)
tzconv_la_CPPFLAGS += -DDECLF=extern -DDEFUN=
tzconv_la_SOURCES = tzconv.c
tzconv_la_SOURCES += tzraw.c tzraw.h
tzconv_la_SOURCES += leaps.c leaps.h
endif  BUILD_MATCLI
EXTRA_DIST += $(mex_DATA)


## get around subdir-object bug in conjunction with distclean in automake 1.14+
tzraw.c tzraw.h:
	$(LN_S) $(top_srcdir)/lib/$@ $@

leaps.c leaps.h:
	$(LN_S) $(top_srcdir)/lib/$@ $@

# 
# Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<
