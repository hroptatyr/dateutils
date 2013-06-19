include $(top_builddir)/version.mk

LANG = C
LC_ALL = C

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib src info test contrib

EXTRA_DIST = build-aux/git-version-gen
DISTCLEANFILES = version.mk

doc_DATA = README.md
EXTRA_DIST += $(doc_DATA)

pkgconfigdir = @libdir@/pkgconfig
pkgconfig_DATA =
pkgconfig_DATA += libdut.pc

dist-hook:
	echo $(VERSION) > $(distdir)/version \
	&& touch -r $(top_builddir)/version.mk $(distdir)/version
