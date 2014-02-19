LANG = C
LC_ALL = C

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=700 -D_BSD_SOURCE
AM_CPPFLAGS += -D_ALL_SOURCE
AM_CPPFLAGS += -D_NETBSD_SOURCE

noinst_PROGRAMS =
BUILT_SOURCES =

if !HAVE_YUCK
noinst_PROGRAMS += yuck-bootstrap
yuck_bootstrap_SOURCES = yuck.c
yuck_bootstrap_CPPFLAGS = $(AM_CPPFLAGS)
yuck_bootstrap_CPPFLAGS += -DBOOTSTRAP

noinst_PROGRAMS += yuck
yuck_SOURCES = yuck.c yuck.yuck
yuck_SOURCES += yuck.m4
yuck_SOURCES += yuck-coru.m4c yuck-coru.m4h
yuck_SOURCES += yuck.m4man
yuck_CPPFLAGS = $(AM_CPPFLAGS)
yuck_CPPFLAGS += -DYUCK_TEMPLATE_PATH='"$(abs_srcdir)"'
BUILT_SOURCES += yuck.yucc
endif  !HAVE_YUCK
EXTRA_DIST = yuck.yucc yuck.m4i
DISTCLEANFILES = yuck.m4i yuck.yucc

## yuck rules
yuck.m4i: yuck.yuck
	$(MAKE) $(AM_MAKEFLAGS) yuck-bootstrap
	$(AM_V_GEN) $(builddir)/yuck-bootstrap$(EXEEXT) $(srcdir)/yuck.yuck > $@ \
		|| { rm -f -- $@; false; }

yuck.yucc: yuck.m4i yuck.m4 yuck-coru.m4h yuck-coru.m4c
	$(AM_V_GEN) $(M4) $(srcdir)/yuck.m4 yuck.m4i \
		$(srcdir)/yuck-coru.m4h $(srcdir)/yuck-coru.m4c > $@ \
		|| { rm -f -- $@; false; }

## Makefile.am ends here
