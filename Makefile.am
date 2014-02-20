include $(top_builddir)/version.mk

LANG = C
LC_ALL = C

ACLOCAL_AMFLAGS = -I m4

SUBDIRS =
EXTRA_DIST =

SUBDIRS += build-aux
SUBDIRS += lib
SUBDIRS += src
SUBDIRS += info
SUBDIRS += test
SUBDIRS += contrib

DISTCLEANFILES = version.mk

doc_DATA = README.md
EXTRA_DIST = $(doc_DATA)


.version:
	$(AM_V_GEN) echo "v$(VERSION)" > $@

dist-hook: .version
	cp $< $(distdir)/.version
