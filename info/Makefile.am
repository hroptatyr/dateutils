# Help the Developers and yourself.  Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG=C
LC_ALL=C

include $(top_builddir)/version.mk

BUILT_SOURCES =
EXTRA_DIST = $(BUILT_SOURCES)
SUFFIXES =

info_TEXINFOS = dateutils.texi
dateutils_TEXINFOS =
dateutils_TEXINFOS += $(built_texis)
dateutils_TEXINFOS += format.texi
dateutils_TEXINFOS += format-ddiff.texi
dateutils_TEXINFOS += units.texi

man1_MANS =
man1_MANS += $(built_mans)
man1_MANS += dateutils.man

dateutils_EXAMPLES =
dateutils_H2M_EX =

dconv_EXAMPLES =
dconv_EXAMPLES += $(top_srcdir)/test/dconv.001.clit
dconv_EXAMPLES += $(top_srcdir)/test/dconv.002.clit
dconv_EXAMPLES += $(top_srcdir)/test/dconv.003.clit
dconv_EXAMPLES += $(top_srcdir)/test/dconv.004.clit
dconv_EXAMPLES += $(top_srcdir)/test/tconv.001.clit
dconv_EXAMPLES += $(top_srcdir)/test/tconv.002.clit
dconv_EXAMPLES += $(top_srcdir)/test/dtconv.001.clit
dconv_EXAMPLES += $(top_srcdir)/test/dtconv.002.clit
dconv_EXAMPLES += $(top_srcdir)/test/dtconv.011.clit
dconv_EXAMPLES += $(top_srcdir)/test/dtconv.012.clit
dconv_EXAMPLES += $(top_srcdir)/test/dtconv.014.clit
dateutils_EXAMPLES += $(dconv_EXAMPLES)
dateutils_H2M_EX += dconv.h2m

dseq_EXAMPLES =
dseq_EXAMPLES += $(top_srcdir)/test/dseq.01.clit
dseq_EXAMPLES += $(top_srcdir)/test/dseq.02.clit
dseq_EXAMPLES += $(top_srcdir)/test/dseq.03.clit
dseq_EXAMPLES += $(top_srcdir)/test/dseq.04.clit
dseq_EXAMPLES += $(top_srcdir)/test/dseq.05.clit
dseq_EXAMPLES += $(top_srcdir)/test/dseq.06.clit
dseq_EXAMPLES += $(top_srcdir)/test/dseq.07.clit
dseq_EXAMPLES += $(top_srcdir)/test/dseq.08.clit
dseq_EXAMPLES += $(top_srcdir)/test/dseq.09.clit
dseq_EXAMPLES += $(top_srcdir)/test/dseq.10.clit
dseq_EXAMPLES += $(top_srcdir)/test/dseq.11.clit
dseq_EXAMPLES += $(top_srcdir)/test/dseq.12.clit
dseq_EXAMPLES += $(top_srcdir)/test/dseq.13.clit
dseq_EXAMPLES += $(top_srcdir)/test/tseq.01.clit
dseq_EXAMPLES += $(top_srcdir)/test/tseq.02.clit
dseq_EXAMPLES += $(top_srcdir)/test/tseq.03.clit
dseq_EXAMPLES += $(top_srcdir)/test/tseq.04.clit
dateutils_EXAMPLES += $(dseq_EXAMPLES)
dateutils_H2M_EX += dseq.h2m

dadd_EXAMPLES =
dadd_EXAMPLES += $(top_srcdir)/test/dadd.001.clit
dadd_EXAMPLES += $(top_srcdir)/test/dadd.002.clit
dadd_EXAMPLES += $(top_srcdir)/test/dadd.003.clit
dadd_EXAMPLES += $(top_srcdir)/test/dadd.004.clit
dadd_EXAMPLES += $(top_srcdir)/test/tadd.001.clit
dadd_EXAMPLES += $(top_srcdir)/test/tadd.002.clit
dadd_EXAMPLES += $(top_srcdir)/test/tadd.003.clit
dateutils_EXAMPLES += $(dadd_EXAMPLES)
dateutils_H2M_EX += dadd.h2m

dtest_EXAMPLES =
dtest_EXAMPLES += $(top_srcdir)/test/dtest.001.clit
dtest_EXAMPLES += $(top_srcdir)/test/dtest.002.clit
dtest_EXAMPLES += $(top_srcdir)/test/dtest.003.clit
dtest_EXAMPLES += $(top_srcdir)/test/dtest.004.clit
dtest_EXAMPLES += $(top_srcdir)/test/dtest.005.clit
dtest_EXAMPLES += $(top_srcdir)/test/ttest.001.clit
dtest_EXAMPLES += $(top_srcdir)/test/ttest.002.clit
dtest_EXAMPLES += $(top_srcdir)/test/ttest.003.clit
dtest_EXAMPLES += $(top_srcdir)/test/ttest.004.clit
dtest_EXAMPLES += $(top_srcdir)/test/dttest.010.clit
dtest_EXAMPLES += $(top_srcdir)/test/dttest.002.clit
dtest_EXAMPLES += $(top_srcdir)/test/dttest.003.clit
dtest_EXAMPLES += $(top_srcdir)/test/dttest.004.clit
dtest_EXAMPLES += $(top_srcdir)/test/dttest.005.clit
dateutils_EXAMPLES += $(dtest_EXAMPLES)
dateutils_H2M_EX += dtest.h2m

ddiff_EXAMPLES =
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.001.clit
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.002.clit
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.003.clit
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.004.clit
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.005.clit
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.006.clit
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.008.clit
ddiff_EXAMPLES += $(top_srcdir)/test/tdiff.001.clit
ddiff_EXAMPLES += $(top_srcdir)/test/tdiff.002.clit
ddiff_EXAMPLES += $(top_srcdir)/test/tdiff.003.clit
ddiff_EXAMPLES += $(top_srcdir)/test/tdiff.008.clit
ddiff_EXAMPLES += $(top_srcdir)/test/tdiff.006.clit
ddiff_EXAMPLES += $(top_srcdir)/test/tdiff.007.clit
ddiff_EXAMPLES += $(top_srcdir)/test/dtdiff.001.clit
ddiff_EXAMPLES += $(top_srcdir)/test/dtdiff.003.clit
ddiff_EXAMPLES += $(top_srcdir)/test/dtdiff.011.clit
ddiff_EXAMPLES += $(top_srcdir)/test/dtdiff.012.clit
dateutils_EXAMPLES += $(ddiff_EXAMPLES)
dateutils_H2M_EX += ddiff.h2m

dgrep_EXAMPLES =
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.001.clit
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.002.clit
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.006.clit
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.007.clit
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.008.clit
dgrep_EXAMPLES += $(top_srcdir)/test/tgrep.001.clit
dgrep_EXAMPLES += $(top_srcdir)/test/tgrep.002.clit
dgrep_EXAMPLES += $(top_srcdir)/test/dtgrep.001.clit
dgrep_EXAMPLES += $(top_srcdir)/test/dtgrep.002.clit
dgrep_EXAMPLES += $(top_srcdir)/test/dtgrep.005.clit
dgrep_EXAMPLES += $(top_srcdir)/test/dtgrep.006.clit
dateutils_EXAMPLES += $(dgrep_EXAMPLES)
dateutils_H2M_EX += dgrep.h2m

dround_EXAMPLES =
dround_EXAMPLES += $(top_srcdir)/test/dround.002.clit
dround_EXAMPLES += $(top_srcdir)/test/dround.003.clit
dround_EXAMPLES += $(top_srcdir)/test/tround.003.clit
dround_EXAMPLES += $(top_srcdir)/test/tround.002.clit
dateutils_EXAMPLES += $(dround_EXAMPLES)
dateutils_H2M_EX += dround.h2m

dzone_EXAMPLES =
dzone_EXAMPLES += $(top_srcdir)/test/dzone.001.clit
dzone_EXAMPLES += $(top_srcdir)/test/dzone.002.clit
dzone_EXAMPLES += $(top_srcdir)/test/dzone.003.clit
dzone_EXAMPLES += $(top_srcdir)/test/dzone.004.clit
dateutils_EXAMPLES += $(dzone_EXAMPLES)
dateutils_H2M_EX += dzone.h2m

EXTRA_DIST += author.h2m
BUILT_SOURCES += format.th2m
BUILT_SOURCES += format-ddiff.th2m
BUILT_SOURCES += units.th2m
BUILT_SOURCES += $(built_texis)
BUILT_SOURCES += $(built_mans)
BUILT_SOURCES += $(dateutils_H2M_EX)


dist_noinst_SCRIPTS = gentexi.sh genh2m.sh texi2h2m.sh

built_texis =
built_texis += strptime.texi
built_texis += dadd.texi
built_texis += dconv.texi
built_texis += ddiff.texi
built_texis += dseq.texi
built_texis += dtest.texi
built_texis += dgrep.texi
built_texis += dround.texi
built_texis += dzone.texi

built_mans =
if HAVE_STRPTIME
built_mans += strptime.man
endif  HAVE_STRPTIME
built_mans += dadd.manu
built_mans += dconv.man
built_mans += ddiff.mand
built_mans += dgrep.man
built_mans += dseq.manu
built_mans += dtest.man
built_mans += dround.manu
built_mans += dzone.man

EXTRA_DIST += $(man1_MANS)
EXTRA_DIST += $(dateutils_TEXINFOS)

EXTRA_DIST += dadd.name
EXTRA_DIST += dconv.name
EXTRA_DIST += ddiff.name
EXTRA_DIST += dgrep.name
EXTRA_DIST += dround.name
EXTRA_DIST += dseq.name
EXTRA_DIST += dtest.name
EXTRA_DIST += dzone.name
EXTRA_DIST += strptime.name

SUFFIXES += .yuck
SUFFIXES += .name
VPATH += @top_srcdir@/src

my_bld_man = $(__bld_man_$(V))
__bld_man_ = $(__bld_man_$(AM_DEFAULT_VERBOSITY))
__bld_man_0 = @echo "  HELP2MAN" $@;
SUFFIXES += .man
.h2m.man: $(dateutils_H2M_EX) $(top_srcdir)/configure
	$(my_bld_man) $(HELP2MAN) -S "$(PACKAGE_STRING)" -p "($(PACKAGE))$*" \
		-I $< \
		-I "$(builddir)/format.h2m" \
		-I "$(srcdir)/author.h2m" \
		-I "$(srcdir)/$*.name" \
		-o $@ $(top_builddir)/src/$*

## generic rule with format-ddiff instead of format
SUFFIXES += .mand
.h2m.mand: $(dateutils_H2M_EX) $(top_srcdir)/configure
	$(my_bld_man) $(HELP2MAN) -S "$(PACKAGE_STRING)" -p "($(PACKAGE))$*" \
		-I $< \
		-I "$(builddir)/format-ddiff.th2m" \
		-I "$(srcdir)/author.h2m" \
		-I "$(srcdir)/$*.name" \
		-o $@ $(top_builddir)/src/$*

## generic rule for man pages with units section
SUFFIXES += .manu
.h2m.manu: $(dateutils_H2M_EX) $(top_srcdir)/configure
	$(my_bld_man) $(HELP2MAN) -S "$(PACKAGE_STRING)" -p "($(PACKAGE))$*" \
		-I $< \
		-I "$(builddir)/format.th2m" \
		-I "$(builddir)/units.th2m" \
		-I "$(srcdir)/author.h2m" \
		-I "$(srcdir)/$*.name" \
		-o $@ $(top_builddir)/src/$*

my_bld_h2m = $(__bld_h2m_$(V))
__bld_h2m_ = $(__bld_h2m_$(AM_DEFAULT_VERBOSITY))
__bld_h2m_0 = @echo "  GENH2M  " $*;
SUFFIXES += .h2m
.yuck.h2m: $(dateutils_EXAMPLES)
	$(my_bld_h2m) ${SHELL} \
		$(srcdir)/genh2m.sh $(top_builddir)/src/$* \
		$($*_EXAMPLES) > $@ || rm -f -- $@

my_bld_texi = $(__bld_texi_$(V))
__bld_texi_ = $(__bld_texi_$(AM_DEFAULT_VERBOSITY))
__bld_texi_0 = @echo "  GENTEXI " $*;
SUFFIXES += .texi
.yuck.texi: $(dateutils_EXAMPLES) dateutils.texi
	$(my_bld_texi) ${SHELL} \
		$(srcdir)/gentexi.sh $(top_builddir)/src/$* \
		$($*_EXAMPLES) > $@ || test "$$?" = "2" || rm -f -- $@

my_bld_th2m = $(__bld_th2m_$(V))
__bld_th2m_ = $(__bld_th2m_$(AM_DEFAULT_VERBOSITY))
__bld_th2m_0 = @echo "  GENTH2M " $*;
SUFFIXES += .th2m
.texi.th2m:
	$(my_bld_th2m) ${SHELL} \
		$(srcdir)/texi2h2m.sh $< > $@ || rm -f -- $@

## Makefile.am ends here
