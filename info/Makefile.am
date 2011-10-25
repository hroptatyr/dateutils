# Help the Developers and yourself.  Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG=C
LC_ALL=C

BUILT_SOURCES =
EXTRA_DIST = $(BUILT_SOURCES)

info_TEXINFOS = dateutils.texi
dateutils_TEXINFOS =
dateutils_TEXINFOS += $(built_texis)
dateutils_TEXINFOS += format.texi

man1_MANS =
man1_MANS += $(built_mans)

dateutils_EXAMPLES =
dateutils_H2M_EX =

dconv_EXAMPLES =
dconv_EXAMPLES += $(top_srcdir)/test/dconv.1.dt
dconv_EXAMPLES += $(top_srcdir)/test/dconv.2.dt
dconv_EXAMPLES += $(top_srcdir)/test/dconv.3.dt
dconv_EXAMPLES += $(top_srcdir)/test/dconv.4.dt
dateutils_EXAMPLES += $(dconv_EXAMPLES)
dateutils_H2M_EX += dconv.h2m

dseq_EXAMPLES =
dseq_EXAMPLES += $(top_srcdir)/test/dseq.1.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.2.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.3.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.4.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.5.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.6.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.7.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.8.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.9.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.10.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.11.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.12.dt
dseq_EXAMPLES += $(top_srcdir)/test/dseq.13.dt
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

dgrep_EXAMPLES =
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.1.dt
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.2.dt
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.6.dt
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.7.dt
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.8.dt
dateutils_EXAMPLES += $(dgrep_EXAMPLES)
dateutils_H2M_EX += dgrep.h2m

tseq_EXAMPLES =
tseq_EXAMPLES += $(top_srcdir)/test/tseq.1.dt
tseq_EXAMPLES += $(top_srcdir)/test/tseq.2.dt
tseq_EXAMPLES += $(top_srcdir)/test/tseq.3.dt
tseq_EXAMPLES += $(top_srcdir)/test/tseq.4.dt
dateutils_EXAMPLES += $(tseq_EXAMPLES)
dateutils_H2M_EX += tseq.h2m

tdiff_EXAMPLES =
tdiff_EXAMPLES += $(top_srcdir)/test/tdiff.1.dt
tdiff_EXAMPLES += $(top_srcdir)/test/tdiff.2.dt
tdiff_EXAMPLES += $(top_srcdir)/test/tdiff.3.dt
dateutils_EXAMPLES += $(tdiff_EXAMPLES)
dateutils_H2M_EX += tdiff.h2m

tadd_EXAMPLES =
tadd_EXAMPLES += $(top_srcdir)/test/tadd.1.dt
tadd_EXAMPLES += $(top_srcdir)/test/tadd.2.dt
tadd_EXAMPLES += $(top_srcdir)/test/tadd.3.dt
dateutils_EXAMPLES += $(tadd_EXAMPLES)
dateutils_H2M_EX += tadd.h2m

tgrep_EXAMPLES =
tgrep_EXAMPLES += $(top_srcdir)/test/tgrep.1.dt
dateutils_EXAMPLES += $(tgrep_EXAMPLES)
dateutils_H2M_EX += tgrep.h2m

ttest_EXAMPLES =
ttest_EXAMPLES += $(top_srcdir)/test/ttest.1.dt
dateutils_EXAMPLES += $(ttest_EXAMPLES)
dateutils_H2M_EX += ttest.h2m

tconv_EXAMPLES =
tconv_EXAMPLES += $(top_srcdir)/test/tconv.1.dt
tconv_EXAMPLES += $(top_srcdir)/test/tconv.2.dt
dateutils_EXAMPLES += $(tconv_EXAMPLES)
dateutils_H2M_EX += tconv.h2m

dateutils_H2M_EX += format.h2m

EXTRA_DIST += author.h2m $(dateutils_H2M_EX)
BUILT_SOURCES += $(built_texis) $(dateutils_H2M_EX) $(built_mans)

dist_noinst_SCRIPTS = gentexi.sh genh2m.sh genex texi2h2m.sh

built_texis =
built_texis += strptime.texi
built_texis += dadd.texi
built_texis += dconv.texi
built_texis += ddiff.texi
built_texis += dseq.texi
built_texis += dtest.texi
built_texis += dgrep.texi
built_texis += tseq.texi
built_texis += tadd.texi
built_texis += tdiff.texi
built_texis += tgrep.texi
built_texis += ttest.texi
built_texis += tconv.texi

built_mans =
built_mans += strptime.man
built_mans += dadd.man
built_mans += dconv.man
built_mans += ddiff.man
built_mans += dgrep.man
built_mans += dseq.man
built_mans += dtest.man
built_mans += tseq.man
built_mans += tadd.man
built_mans += tdiff.man
built_mans += tgrep.man
built_mans += ttest.man
built_mans += tconv.man

EXTRA_DIST += $(man1_MANS)
EXTRA_DIST += $(dateutils_TEXINFOS)

my_bld_man = $(__bld_man_$(V))
__bld_man_ = $(__bld_man_$(AM_DEFAULT_VERBOSITY))
__bld_man_0 = @echo "  HELP2MAN" $*;
%.man: $(top_srcdir)/src/%-clo.ggo $(dateutils_H2M_EX)
	$(my_bld_man) $(HELP2MAN) -S "$(PACKAGE_STRING)" -p "($(PACKAGE))$*" \
		-I "$(srcdir)/$*.h2m" -I "$(builddir)/$*.h2m" \
		-I "$(srcdir)/format.h2m" -I "$(builddir)/format.h2m" \
		-I "$(srcdir)/author.h2m" \
		-o $@ $(top_builddir)/src/$*

my_bld_h2m = $(__bld_h2m_$(V))
__bld_h2m_ = $(__bld_h2m_$(AM_DEFAULT_VERBOSITY))
__bld_h2m_0 = @echo "  GENH2M  " $*;
%.h2m: $(top_srcdir)/src/%-clo.ggo $(dateutils_EXAMPLES)
	$(my_bld_h2m) ${SHELL} \
		$(srcdir)/genh2m.sh $(top_builddir)/src/$* \
		$($*_EXAMPLES) > $@ || rm -f -- $@

my_bld_texi = $(__bld_texi_$(V))
__bld_texi_ = $(__bld_texi_$(AM_DEFAULT_VERBOSITY))
__bld_texi_0 = @echo "  GENTEXI " $*;
%.texi: $(top_srcdir)/src/%-clo.ggo $(dateutils_EXAMPLES) dateutils.texi
	$(my_bld_texi) ${SHELL} \
		$(srcdir)/gentexi.sh $(top_builddir)/src/$* \
		$($*_EXAMPLES) > $@ || rm -f -- $@

%.h2m: %.texi
	$(my_bld_h2m) ${SHELL} \
		$(srcdir)/texi2h2m.sh $< > $@ || rm -f -- $@

## Makefile.am ends here