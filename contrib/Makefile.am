### Makefile.am

AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_LDFLAGS =
EXTRA_DIST =

MEX_LDFLAGS = -module -shared -avoid-version -shrext ".mexa64"
OCT_LDFLAGS = -module -shared -avoid-version -shrext ".oct"

if BUILD_MEXCLI
mexdir = $(MATLABPATH)/dateutils/
mex_LTLIBRARIES =
mex_DATA =

mex_LTLIBRARIES += tzconv-matlab.la
mex_DATA += tzconv.m
tzconv_matlab_la_LDFLAGS = $(AM_LDFLAGS)
tzconv_matlab_la_LDFLAGS += $(MEX_LDFLAGS)
tzconv_matlab_la_LDFLAGS += $(XCCLDFLAGS)
tzconv_matlab_la_CPPFLAGS = $(AM_CPPFLAGS) -DMATLAB_MEX_FILE
tzconv_matlab_la_CPPFLAGS += -I$(top_srcdir)/lib
tzconv_matlab_la_CPPFLAGS += -I$(top_builddir)/lib
tzconv_matlab_la_CPPFLAGS += $(matlab_CFLAGS)
tzconv_matlab_la_CPPFLAGS += -DDECLF=extern -DDEFUN=
tzconv_matlab_la_SOURCES = tzconv.c
tzconv_matlab_la_SOURCES += tzraw.c tzraw.h
tzconv_matlab_la_SOURCES += leaps.c leaps.h
endif  BUILD_MEXCLI
EXTRA_DIST += $(mex_DATA)

if BUILD_OCTCLI
octdir = $(OCTAVEPATH)/dateutils/
oct_LTLIBRARIES =
oct_DATA =

oct_LTLIBRARIES += tzconv-octave.la
oct_DATA += tzconv.m
tzconv_octave_la_LDFLAGS = $(AM_LDFLAGS)
tzconv_octave_la_LDFLAGS += $(OCT_LDFLAGS)
tzconv_octave_la_LDFLAGS += $(XCCLDFLAGS)
tzconv_octave_la_CPPFLAGS = $(AM_CPPFLAGS) -DOCTAVE_OCT_FILE
tzconv_octave_la_CPPFLAGS += -I$(top_srcdir)/lib
tzconv_octave_la_CPPFLAGS += -I$(top_builddir)/lib
tzconv_octave_la_CPPFLAGS += $(octave_CFLAGS)
tzconv_octave_la_CPPFLAGS += -DDECLF=extern -DDEFUN=
tzconv_octave_la_SOURCES = tzconv.c
tzconv_octave_la_SOURCES += tzraw.c tzraw.h
tzconv_octave_la_SOURCES += leaps.c leaps.h
endif  BUILD_OCTCLI


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
