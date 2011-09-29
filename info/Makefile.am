# Help the Developers and yourself.  Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG=C
LC_ALL=C

info_TEXINFOS = dateutils.texi
dateutils_TEXINFOS =
dateutils_TEXINFOS += strptime.texi
dateutils_TEXINFOS += dadd.texi
dateutils_TEXINFOS += dcal.texi
dateutils_TEXINFOS += ddiff.texi
dateutils_TEXINFOS += dseq.texi
dateutils_TEXINFOS += dtest.texi

dateutils_EXAMPLES =
dateutils_H2M_EX =

dcal_EXAMPLES =
dcal_EXAMPLES += $(top_srcdir)/test/dcal.1.dt
dcal_EXAMPLES += $(top_srcdir)/test/dcal.2.dt
dcal_EXAMPLES += $(top_srcdir)/test/dcal.3.dt
dcal_EXAMPLES += $(top_srcdir)/test/dcal.4.dt
dateutils_EXAMPLES += $(dcal_EXAMPLES)
dateutils_H2M_EX += dcal.h2m

dseq_EXAMPLES =
dseq_EXAMPLES += $(top_srcdir)/test/dseq.1.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.2.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.3.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.4.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.5.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.6.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.7.dt
dateutils_EXAMPLES += $(dseq_EXAMPLES)
dateutils_H2M_EX += dseq.h2m

dadd_EXAMPLES =
dadd_EXAMPLES += $(top_srcdir)/test/dadd.1.dt
dadd_EXAMPLES += $(top_srcdir)/test/dadd.2.dt
dadd_EXAMPLES += $(top_srcdir)/test/dadd.3.dt
dadd_EXAMPLES += $(top_srcdir)/test/dadd.4.dt
dateutils_EXAMPLES += $(dadd_EXAMPLES)
dateutils_H2M_EX += dadd.h2m

dtest_EXAMPLES =
dtest_EXAMPLES += $(top_srcdir)/test/dtest.1.dt
dtest_EXAMPLES += $(top_srcdir)/test/dtest.2.dt
dtest_EXAMPLES += $(top_srcdir)/test/dtest.3.dt
dtest_EXAMPLES += $(top_srcdir)/test/dtest.4.dt
dtest_EXAMPLES += $(top_srcdir)/test/dtest.5.dt
dateutils_EXAMPLES += $(dtest_EXAMPLES)
dateutils_H2M_EX += dtest.h2m

ddiff_EXAMPLES =
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.1.dt
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.2.dt
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.3.dt
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.4.dt
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.5.dt
dateutils_EXAMPLES += $(ddiff_EXAMPLES)
dateutils_H2M_EX += ddiff.h2m

BUILT_SOURCES = $(dateutils_TEXINFOS) $(dateutils_H2M_EX)

TEXI2DVI_TEX = latex
TEXI2DVI = TEX=$(TEXI2DVI_TEX) texi2dvi -c -s

dist_noinst_SCRIPTS = gentexi.sh genh2m.sh genex

man1_MANS = $(dateutils_TEXINFOS:.texi=.man)

my_bld_man = $(__bld_man_$(V))
__bld_man_ = $(__bld_man_$(AM_DEFAULT_VERBOSITY))
__bld_man_0 = @echo "  HELP2MAN" $*;
%.man: $(top_builddir)/src/%$(EXEEXT) $(dateutils_H2M_EX) format.h2m
	$(my_bld_man) $(HELP2MAN) -S "$(PACKAGE_STRING)" -p "($(PACKAGE))$*" \
		-I "$(srcdir)/$*.h2m" -I "$(builddir)/$*.h2m" \
		-I "$(srcdir)/format.h2m" \
		-o "$@" "$<"

my_bld_h2m = $(__bld_h2m_$(V))
__bld_h2m_ = $(__bld_h2m_$(AM_DEFAULT_VERBOSITY))
__bld_h2m_0 = @echo "  GENH2M  " $*;
%.h2m: $(top_builddir)/src/%$(EXEEXT) $(dateutils_EXAMPLES)
	$(my_bld_h2m) ${SHELL} \
		$(srcdir)/genh2m.sh $< $($*_EXAMPLES) > $@ || rm -f -- $@

my_bld_texi = $(__bld_texi_$(V))
__bld_texi_ = $(__bld_texi_$(AM_DEFAULT_VERBOSITY))
__bld_texi_0 = @echo "  GENTEXI " $*;
%.texi: $(top_builddir)/src/%$(EXEEXT) $(dateutils_EXAMPLES) dateutils.texi
	$(my_bld_texi) ${SHELL} \
		$(srcdir)/gentexi.sh $< $($*_EXAMPLES) > $@ || rm -f -- $@

## Makefile.am ends here
