include $(top_builddir)/version.mk

LANG = C
LC_ALL = C

ACLOCAL_AMFLAGS = -I m4

SUBDIRS =
EXTRA_DIST = $(DISTCLEANFILES) $(doc_DATA)
DISTCLEANFILES =
CLEANFILES =
doc_DATA =

SUBDIRS += build-aux
SUBDIRS += data
SUBDIRS += lib
SUBDIRS += src
SUBDIRS += info
SUBDIRS += test
if BUILD_MEXCLI
SUBDIRS += contrib
endif  BUILD_MEXCLI
if BUILD_OCTCLI
SUBDIRS += contrib
endif  BUILD_OCTCLI

DISTCLEANFILES += version.mk
DISTCLEANFILES += .version
EXTRA_DIST += version.mk.in

doc_DATA += README.md
doc_DATA += LICENSE

## make sure .version is read-only in the dist
dist-hook:
	chmod ugo-w $(distdir)/.version

## Makefile.am ends here
