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

datetools_EXAMPLES =
datetools_H2M_EX =

dcal_EXAMPLES =
dcal_EXAMPLES += $(top_srcdir)/test/dcal.1.dt
dcal_EXAMPLES += $(top_srcdir)/test/dcal.2.dt
dcal_EXAMPLES += $(top_srcdir)/test/dcal.3.dt
dcal_EXAMPLES += $(top_srcdir)/test/dcal.4.dt
datetools_EXAMPLES += $(dcal_EXAMPLES)
datetools_H2M_EX += dcal.h2m

dseq_EXAMPLES =
dseq_EXAMPLES += $(top_srcdir)/test/dseq.1.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.2.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.3.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.4.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.5.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.6.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.7.dt
datetools_EXAMPLES += $(dseq_EXAMPLES)
datetools_H2M_EX += dseq.h2m

BUILT_SOURCES = $(datetools_TEXINFOS) $(datetools_H2M_EX)

TEXI2DVI_TEX = latex
TEXI2DVI = TEX=$(TEXI2DVI_TEX) texi2dvi -c -s

noinst_SCRIPTS = gentexi.sh genh2m.sh genex

man1_MANS = $(datetools_TEXINFOS:.texi=.man)

my_bld_man = $(__bld_man_$(V))
__bld_man_ = $(__bld_man_$(AM_DEFAULT_VERBOSITY))
__bld_man_0 = @echo "  HELP2MAN" $*;
%.man: $(top_builddir)/src/%$(EXEEXT) $(datetools_H2M_EX)
	$(my_bld_man) $(HELP2MAN) -S "$(PACKAGE_STRING)" -p "($(PACKAGE))$*" \
		-I "$(srcdir)/$*.h2m" -I "$(builddir)/$*.h2m" -o "$@" "$<"

my_bld_h2m = $(__bld_h2m_$(V))
__bld_h2m_ = $(__bld_h2m_$(AM_DEFAULT_VERBOSITY))
__bld_h2m_0 = @echo "  GENH2M  " $*;
%.h2m: $(top_builddir)/src/%$(EXEEXT) $(datetools_EXAMPLES)
	$(my_bld_h2m) ${SHELL} \
		$(srcdir)/genh2m.sh $< $($*_EXAMPLES) > $@ || rm -f -- $@

my_bld_texi = $(__bld_texi_$(V))
__bld_texi_ = $(__bld_texi_$(AM_DEFAULT_VERBOSITY))
__bld_texi_0 = @echo "  GENTEXI " $*;
%.texi: $(top_builddir)/src/%$(EXEEXT) $(datetools_EXAMPLES) datetools.texi
	$(my_bld_texi) ${SHELL} \
		$(srcdir)/gentexi.sh $< $($*_EXAMPLES) > $@ || rm -f -- $@

## Makefile.am ends here
