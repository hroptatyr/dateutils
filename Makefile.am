include $(top_builddir)/version.mk

LANG = C
LC_ALL = C

ACLOCAL_AMFLAGS = -I m4

SUBDIRS =
EXTRA_DIST = $(BUILT_SOURCES)
BUILT_SOURCES =

SUBDIRS += build-aux
SUBDIRS += lib
SUBDIRS += src
SUBDIRS += info
SUBDIRS += test
SUBDIRS += contrib

DISTCLEANFILES = version.mk
EXTRA_DIST += version.mk
EXTRA_DIST += version.mk.in

doc_DATA = README.md
EXTRA_DIST += $(doc_DATA)

BUILT_SOURCES += .version

.version:
	$(AM_V_GEN) echo "v$(VERSION)" > $@

## Makefile.am ends here
