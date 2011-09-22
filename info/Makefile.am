# Help the Developers and yourself.  Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG=C
LC_ALL=C

info_TEXINFOS = datetools.texi
datetools_TEXINFOS =
datetools_TEXINFOS += strptime.texi
datetools_TEXINFOS += dadd.texi
datetools_TEXINFOS += dcal.texi
datetools_TEXINFOS += ddiff.texi
datetools_TEXINFOS += dseq.texi
datetools_TEXINFOS += dtest.texi

BUILT_SOURCES = $(datetools_TEXINFOS)

TEXI2DVI_TEX = latex
TEXI2DVI = TEX=$(TEXI2DVI_TEX) texi2dvi -c -s

noinst_SCRIPTS = gentexi.sh

man1_MANS = $(datetools_TEXINFOS:.texi=.man)

my_bld_man = $(__bld_man_$(V))
__bld_man_ = $(__bld_man_$(AM_DEFAULT_VERBOSITY))
__bld_man_0 = @echo "  HELP2MAN" $*;
%.man: $(top_builddir)/src/%$(EXEEXT)
	$(my_bld_man) $(HELP2MAN) -S "$(PACKAGE_STRING)" -p "($(PACKAGE))$*" \
		-I "$(srcdir)/$*.h2m" -I "$(builddir)/$*.h2m" -o "$@" "$<"

my_bld_texi = $(__bld_texi_$(V))
__bld_texi_ = $(__bld_texi_$(AM_DEFAULT_VERBOSITY))
__bld_texi_0 = @echo "  GENTEXI " $*;
%.texi: $(top_builddir)/src/%$(EXEEXT) datetools.texi
	$(my_bld_texi) ${SHELL} $(srcdir)/gentexi.sh "$<" > "$@" || rm -f -- "${@}"

## Makefile.am ends here
