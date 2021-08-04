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
dateutils_TEXINFOS += zone.texi
dateutils_TEXINFOS += locale.texi

man1_MANS =
man1_MANS += $(built_mans)
man1_MANS += dateutils.man

dateutils_EXAMPLES =
dateutils_H2M_EX =

dconv_EXAMPLES =
dconv_EXAMPLES += $(top_srcdir)/test/dconv.001.ctst
dconv_EXAMPLES += $(top_srcdir)/test/dconv.002.ctst
dconv_EXAMPLES += $(top_srcdir)/test/dconv.003.ctst
dconv_EXAMPLES += $(top_srcdir)/test/dconv.004.ctst
dconv_EXAMPLES += $(top_srcdir)/test/tconv.001.ctst
dconv_EXAMPLES += $(top_srcdir)/test/tconv.002.ctst
dconv_EXAMPLES += $(top_srcdir)/test/dtconv.001.ctst
dconv_EXAMPLES += $(top_srcdir)/test/dtconv.002.ctst
dconv_EXAMPLES += $(top_srcdir)/test/dtconv.011.ctst
dconv_EXAMPLES += $(top_srcdir)/test/dtconv.012.ctst
dconv_EXAMPLES += $(top_srcdir)/test/dtconv.014.ctst
dateutils_EXAMPLES += $(dconv_EXAMPLES)
dateutils_H2M_EX += dconv.h2m

dseq_EXAMPLES =
dseq_EXAMPLES += $(top_srcdir)/test/dseq.01.ctst
dseq_EXAMPLES += $(top_srcdir)/test/dseq.02.ctst
dseq_EXAMPLES += $(top_srcdir)/test/dseq.03.ctst
dseq_EXAMPLES += $(top_srcdir)/test/dseq.04.ctst
dseq_EXAMPLES += $(top_srcdir)/test/dseq.05.ctst
dseq_EXAMPLES += $(top_srcdir)/test/dseq.06.ctst
dseq_EXAMPLES += $(top_srcdir)/test/dseq.07.ctst
dseq_EXAMPLES += $(top_srcdir)/test/dseq.08.ctst
dseq_EXAMPLES += $(top_srcdir)/test/dseq.09.ctst
dseq_EXAMPLES += $(top_srcdir)/test/dseq.10.ctst
dseq_EXAMPLES += $(top_srcdir)/test/dseq.11.ctst
dseq_EXAMPLES += $(top_srcdir)/test/dseq.12.ctst
dseq_EXAMPLES += $(top_srcdir)/test/dseq.13.ctst
dseq_EXAMPLES += $(top_srcdir)/test/tseq.01.ctst
dseq_EXAMPLES += $(top_srcdir)/test/tseq.02.ctst
dseq_EXAMPLES += $(top_srcdir)/test/tseq.03.ctst
dseq_EXAMPLES += $(top_srcdir)/test/tseq.04.ctst
dateutils_EXAMPLES += $(dseq_EXAMPLES)
dateutils_H2M_EX += dseq.h2m

dadd_EXAMPLES =
dadd_EXAMPLES += $(top_srcdir)/test/dadd.001.ctst
dadd_EXAMPLES += $(top_srcdir)/test/dadd.002.ctst
dadd_EXAMPLES += $(top_srcdir)/test/dadd.003.ctst
dadd_EXAMPLES += $(top_srcdir)/test/dadd.004.ctst
dadd_EXAMPLES += $(top_srcdir)/test/tadd.001.ctst
dadd_EXAMPLES += $(top_srcdir)/test/tadd.002.ctst
dadd_EXAMPLES += $(top_srcdir)/test/tadd.003.ctst
dateutils_EXAMPLES += $(dadd_EXAMPLES)
dateutils_H2M_EX += dadd.h2m

dtest_EXAMPLES =
dtest_EXAMPLES += $(top_srcdir)/test/dtest.001.ctst
dtest_EXAMPLES += $(top_srcdir)/test/dtest.002.ctst
dtest_EXAMPLES += $(top_srcdir)/test/dtest.003.ctst
dtest_EXAMPLES += $(top_srcdir)/test/dtest.004.ctst
dtest_EXAMPLES += $(top_srcdir)/test/dtest.005.ctst
dtest_EXAMPLES += $(top_srcdir)/test/ttest.001.ctst
dtest_EXAMPLES += $(top_srcdir)/test/ttest.002.ctst
dtest_EXAMPLES += $(top_srcdir)/test/ttest.003.ctst
dtest_EXAMPLES += $(top_srcdir)/test/ttest.004.ctst
dtest_EXAMPLES += $(top_srcdir)/test/dttest.010.ctst
dtest_EXAMPLES += $(top_srcdir)/test/dttest.002.ctst
dtest_EXAMPLES += $(top_srcdir)/test/dttest.003.ctst
dtest_EXAMPLES += $(top_srcdir)/test/dttest.004.ctst
dtest_EXAMPLES += $(top_srcdir)/test/dttest.005.ctst
dateutils_EXAMPLES += $(dtest_EXAMPLES)
dateutils_H2M_EX += dtest.h2m

ddiff_EXAMPLES =
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.001.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.002.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.003.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.004.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.005.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.006.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/ddiff.008.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/tdiff.001.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/tdiff.002.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/tdiff.003.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/tdiff.008.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/tdiff.006.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/tdiff.007.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/dtdiff.001.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/dtdiff.003.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/dtdiff.011.ctst
ddiff_EXAMPLES += $(top_srcdir)/test/dtdiff.012.ctst
dateutils_EXAMPLES += $(ddiff_EXAMPLES)
dateutils_H2M_EX += ddiff.h2m

dgrep_EXAMPLES =
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.001.ctst
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.002.ctst
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.006.ctst
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.007.ctst
dgrep_EXAMPLES += $(top_srcdir)/test/dgrep.008.ctst
dgrep_EXAMPLES += $(top_srcdir)/test/tgrep.001.ctst
dgrep_EXAMPLES += $(top_srcdir)/test/tgrep.002.ctst
dgrep_EXAMPLES += $(top_srcdir)/test/dtgrep.001.ctst
dgrep_EXAMPLES += $(top_srcdir)/test/dtgrep.002.ctst
dgrep_EXAMPLES += $(top_srcdir)/test/dtgrep.005.ctst
dgrep_EXAMPLES += $(top_srcdir)/test/dtgrep.006.ctst
dateutils_EXAMPLES += $(dgrep_EXAMPLES)
dateutils_H2M_EX += dgrep.h2m

dround_EXAMPLES =
dround_EXAMPLES += $(top_srcdir)/test/dround.002.ctst
dround_EXAMPLES += $(top_srcdir)/test/dround.003.ctst
dround_EXAMPLES += $(top_srcdir)/test/tround.003.ctst
dround_EXAMPLES += $(top_srcdir)/test/tround.002.ctst
dateutils_EXAMPLES += $(dround_EXAMPLES)
dateutils_H2M_EX += dround.h2m

dzone_EXAMPLES =
dzone_EXAMPLES += $(top_srcdir)/test/dzone.001.ctst
dzone_EXAMPLES += $(top_srcdir)/test/dzone.002.ctst
dzone_EXAMPLES += $(top_srcdir)/test/dzone.003.ctst
dzone_EXAMPLES += $(top_srcdir)/test/dzone.004.ctst
dateutils_EXAMPLES += $(dzone_EXAMPLES)
dateutils_H2M_EX += dzone.h2m

dsort_EXAMPLES =
dsort_EXAMPLES += $(top_srcdir)/test/dsort.001.ctst
dsort_EXAMPLES += $(top_srcdir)/test/dsort.002.ctst
dsort_EXAMPLES += $(top_srcdir)/test/dsort.003.ctst
dsort_EXAMPLES += $(top_srcdir)/test/dsort.004.ctst
dsort_EXAMPLES += $(top_srcdir)/test/dsort.005.ctst
dsort_EXAMPLES += $(top_srcdir)/test/dsort.006.ctst
dateutils_EXAMPLES += $(dsort_EXAMPLES)
dateutils_H2M_EX += dsort.h2m

if HAVE_STRPTIME
strptime_EXAMPLES =
strptime_EXAMPLES += $(top_srcdir)/test/strptime.001.ctst
strptime_EXAMPLES += $(top_srcdir)/test/strptime.002.ctst
dateutils_EXAMPLES += $(strptime_EXAMPLES)
dateutils_H2M_EX += strptime.h2m
endif  HAVE_STRPTIME

EXTRA_DIST += author.h2m
BUILT_SOURCES += format.th2m
BUILT_SOURCES += format-ddiff.th2m
BUILT_SOURCES += units.th2m
BUILT_SOURCES += $(dateutils_H2M_EX)
BUILT_SOURCES += $(built_texis)
BUILT_SOURCES += $(built_mans)


dist_noinst_SCRIPTS = gentexi.sh genh2m.sh texi2h2m.sh

built_texis =
built_texis += strptime.texi
BUILT_SOURCES += dadd.texi
BUILT_SOURCES += dconv.texi
BUILT_SOURCES += ddiff.texi
BUILT_SOURCES += dgrep.texi
BUILT_SOURCES += dround.texi
BUILT_SOURCES += dseq.texi
BUILT_SOURCES += dsort.texi
BUILT_SOURCES += dtest.texi
BUILT_SOURCES += dzone.texi
built_texis += dateadd.texi
built_texis += dateconv.texi
built_texis += datediff.texi
built_texis += dategrep.texi
built_texis += dateround.texi
built_texis += dateseq.texi
built_texis += datesort.texi
built_texis += datetest.texi
built_texis += datezone.texi

built_mans =
if HAVE_STRPTIME
built_mans += strptime.man
endif  HAVE_STRPTIME

if WITH_OLD_NAMES
built_mans += dadd.manu
built_mans += dconv.man
built_mans += ddiff.mand
built_mans += dgrep.man
built_mans += dround.manu
built_mans += dseq.manu
built_mans += dsort.man
built_mans += dtest.man
built_mans += dzone.man
else  !WITH_OLD_NAMES
built_mans += dateadd.manu
built_mans += dateconv.man
built_mans += datediff.mand
built_mans += dategrep.man
built_mans += dateround.manu
built_mans += dateseq.manu
built_mans += datesort.man
built_mans += datetest.man
built_mans += datezone.man
endif  !WITH_OLD_NAMES

if WITH_OLD_LINKS
## install them as files, not links
built_mans += dadd.manu
built_mans += dconv.man
built_mans += ddiff.mand
built_mans += dgrep.man
built_mans += dround.manu
built_mans += dseq.manu
built_mans += dsort.man
built_mans += dtest.man
built_mans += dzone.man
endif  WITH_OLD_LINKS

EXTRA_DIST += $(man1_MANS)
EXTRA_DIST += $(dateutils_TEXINFOS)

## just to aid the suffix rules
strptime.man: strptime.h2m
dadd.manu: dadd.h2m
dconv.man: dconv.h2m
ddiff.mand: ddiff.h2m
dgrep.man: dgrep.h2m
dround.manu: dround.h2m
dseq.manu: dseq.h2m
dsort.man: dsort.h2m
dtest.man: dtest.h2m
dzone.man: dzone.h2m

strptime.h2m: $(strptime_EXAMPLES)
dadd.h2m: $(dadd_EXAMPLES)
dconv.h2m: $(dconv_EXAMPLES)
ddiff.h2m: $(ddiff_EXAMPLES)
dgrep.h2m: $(dgrep_EXAMPLES)
dround.h2m: $(dround_EXAMPLES)
dseq.h2m: $(dseq_EXAMPLES)
dsort.h2m: $(dsort_EXAMPLES)
dtest.h2m: $(dtest_EXAMPLES)
dzone.h2m: $(dzone_EXAMPLES)

strptime.texi: $(strptime_EXAMPLES) dateutils.texi
dadd.texi: $(dadd_EXAMPLES) dateutils.texi
dconv.texi: $(dconv_EXAMPLES) dateutils.texi
ddiff.texi: $(ddiff_EXAMPLES) dateutils.texi
dgrep.texi: $(dgrep_EXAMPLES) dateutils.texi
dround.texi: $(dround_EXAMPLES) dateutils.texi
dseq.texi: $(dseq_EXAMPLES) dateutils.texi
dsort.texi: $(dsort_EXAMPLES) dateutils.texi
dtest.texi: $(dtest_EXAMPLES) dateutils.texi
dzone.texi: $(dzone_EXAMPLES) dateutils.texi

## new file names
TRAFO = sed 's/dadd/dateadd/g; s/dconv/dateconv/g; s/ddiff/datediff/g; s/dgrep/dategrep/g; s/dround/dateround/g; s/dseq/dateseq/g; s/dsort/datesort/g; s/dtest/datetest/g; s/dzone/datezone/g'

dateadd.manu: dadd.manu
	$(TRAFO) < dadd.manu > $@
dateconv.man: dconv.man
	$(TRAFO) < dconv.man > $@
datediff.mand: ddiff.mand
	$(TRAFO) < ddiff.mand > $@
dategrep.man: dgrep.man
	$(TRAFO) < dgrep.man > $@
dateround.manu: dround.manu
	$(TRAFO) < dround.manu > $@
dateseq.manu: dseq.manu
	$(TRAFO) < dseq.manu > $@
datesort.man: dsort.man
	$(TRAFO) < dsort.man > $@
datetest.man: dtest.man
	$(TRAFO) < dtest.man > $@
datezone.man: dzone.man
	$(TRAFO) < dzone.man > $@

dateadd.texi: dadd.texi
	$(TRAFO) < dadd.texi > $@
dateconv.texi: dconv.texi
	$(TRAFO) < dconv.texi > $@
datediff.texi: ddiff.texi
	$(TRAFO) < ddiff.texi > $@
dategrep.texi: dgrep.texi
	$(TRAFO) < dgrep.texi > $@
dateround.texi: dround.texi
	$(TRAFO) < dround.texi > $@
dateseq.texi: dseq.texi
	$(TRAFO) < dseq.texi > $@
datesort.texi: dsort.texi
	$(TRAFO) < dsort.texi > $@
datetest.texi: dtest.texi
	$(TRAFO) < dtest.texi > $@
datezone.texi: dzone.texi
	$(TRAFO) < dzone.texi > $@


SUFFIXES += .yuck
VPATH = $(srcdir):@top_srcdir@/src

SUFFIXES += .man
.yuck.man:
	$(AM_V_GEN) PATH="$(top_builddir)/build-aux:$${PATH}" \
		$(YUCK) genman \
			--package "$(PACKAGE_NAME)" \
			--info-page \
			-i "$(builddir)/format.th2m" \
			-i "$(builddir)/$*.h2m" \
			-i "$(srcdir)/author.h2m" \
			-o $@ --version-file "$(top_builddir)/.version" $<

## generic rule with format-ddiff instead of format
SUFFIXES += .mand
.yuck.mand:
	$(AM_V_GEN) PATH="$(top_builddir)/build-aux:$${PATH}" \
		$(YUCK) genman \
			--package "$(PACKAGE_NAME)" \
			--info-page \
			-i "$(builddir)/format.th2m" \
			-i "$(builddir)/format-ddiff.th2m" \
			-i "$(builddir)/$*.h2m" \
			-i "$(srcdir)/author.h2m" \
			-o $@ --version-file "$(top_builddir)/.version" $<

## generic rule for man pages with units section
SUFFIXES += .manu
.yuck.manu:
	$(AM_V_GEN) PATH="$(top_builddir)/build-aux:$${PATH}" \
		$(YUCK) genman \
			--package "$(PACKAGE_NAME)" \
			--info-page \
			-i "$(builddir)/format.th2m" \
			-i "$(builddir)/units.th2m" \
			-i "$(builddir)/$*.h2m" \
			-i "$(srcdir)/author.h2m" \
			-o $@ --version-file "$(top_builddir)/.version" $<

my_bld_h2m = $(__bld_h2m_$(V))
__bld_h2m_ = $(__bld_h2m_$(AM_DEFAULT_VERBOSITY))
__bld_h2m_0 = @echo "  GENH2M  " $*;
SUFFIXES += .h2m
.yuck.h2m:
	$(my_bld_h2m) ${SHELL} \
		$(srcdir)/genh2m.sh $(top_builddir)/src/$* \
		$($*_EXAMPLES) > $@ || rm -f -- $@

my_bld_texi = $(__bld_texi_$(V))
__bld_texi_ = $(__bld_texi_$(AM_DEFAULT_VERBOSITY))
__bld_texi_0 = @echo "  GENTEXI " $*;
SUFFIXES += .texi
.yuck.texi:
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
