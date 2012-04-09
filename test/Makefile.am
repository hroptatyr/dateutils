# Help the Developers and yourself. Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG = C
LC_ALL = C

AM_CPPFLAGS = -DTEST -I$(abs_top_srcdir)/lib

EXTRA_DIST = $(dt_tests) $(DT_LOG_COMPILER)
TESTS = $(bin_tests) $(dt_tests)
dt_tests =
bin_tests =
check_PROGRAMS =

TEST_EXTENSIONS = .dt
DT_LOG_COMPILER = $(srcdir)/dt-test.sh
AM_DT_LOG_FLAGS = --builddir $(top_builddir)/src --hash sha1sum

dt_tests += dseq.1.dt
dt_tests += dseq.2.dt
dt_tests += dseq.3.dt
dt_tests += dseq.4.dt
dt_tests += dseq.5.dt
dt_tests += dseq.6.dt
dt_tests += dseq.7.dt
dt_tests += dseq.8.dt
dt_tests += dseq.9.dt
dt_tests += dseq.10.dt
dt_tests += dseq.11.dt
dt_tests += dseq.12.dt
dt_tests += dseq.13.dt
dt_tests += dseq.14.dt
dt_tests += dseq.15.dt
dt_tests += dseq.16.dt
dt_tests += dseq.17.dt
dt_tests += dseq.18.dt

dt_tests += dconv.1.dt
dt_tests += dconv.2.dt
dt_tests += dconv.3.dt
dt_tests += dconv.4.dt
dt_tests += dconv.5.dt
dt_tests += dconv.6.dt
dt_tests += dconv.7.dt
dt_tests += dconv.8.dt
dt_tests += dconv.9.dt
dt_tests += dconv.10.dt
dt_tests += dconv.11.dt
dt_tests += dconv.12.dt
dt_tests += dconv.13.dt
dt_tests += dconv.14.dt
dt_tests += dconv.15.dt
dt_tests += dconv.16.dt
dt_tests += dconv.17.dt
dt_tests += dconv.18.dt
dt_tests += dconv.19.dt
dt_tests += dconv.20.dt
dt_tests += dconv.21.dt
dt_tests += dconv.22.dt
dt_tests += dconv.23.dt
dt_tests += dconv.24.dt
dt_tests += dconv.25.dt
dt_tests += dconv.26.dt
dt_tests += dconv.27.dt
dt_tests += dconv.28.dt
dt_tests += dconv.29.dt
dt_tests += dconv.30.dt

dt_tests += dadd.1.dt
dt_tests += dadd.2.dt
dt_tests += dadd.3.dt
dt_tests += dadd.4.dt
dt_tests += dadd.5.dt
dt_tests += dadd.6.dt
dt_tests += dadd.7.dt
dt_tests += dadd.8.dt
dt_tests += dadd.9.dt
dt_tests += dadd.10.dt
dt_tests += dadd.11.dt
dt_tests += dadd.12.dt
dt_tests += dadd.13.dt
dt_tests += dadd.14.dt
dt_tests += dadd.15.dt
dt_tests += dadd.16.dt
dt_tests += dadd.17.dt
dt_tests += dadd.18.dt
dt_tests += dadd.19.dt
dt_tests += dadd.20.dt
dt_tests += dadd.21.dt
dt_tests += dadd.22.dt
dt_tests += dadd.23.dt

dt_tests += dtest.1.dt
dt_tests += dtest.2.dt
dt_tests += dtest.3.dt
dt_tests += dtest.4.dt
dt_tests += dtest.5.dt

dt_tests += ddiff.1.dt
dt_tests += ddiff.2.dt
dt_tests += ddiff.3.dt
dt_tests += ddiff.4.dt
dt_tests += ddiff.5.dt
dt_tests += ddiff.6.dt
dt_tests += ddiff.7.dt
dt_tests += ddiff.8.dt

dt_tests += dgrep.1.dt
dt_tests += dgrep.2.dt
dt_tests += dgrep.3.dt
dt_tests += dgrep.4.dt
dt_tests += dgrep.5.dt
dt_tests += dgrep.6.dt
dt_tests += dgrep.7.dt
dt_tests += dgrep.8.dt
dt_tests += dgrep.9.dt
dt_tests += dgrep.10.dt
dt_tests += dgrep.11.dt
#dt_tests += dgrep.12.dt
dt_tests += dgrep.13.dt
#dt_tests += dgrep.14.dt
dt_tests += dgrep.15.dt
dt_tests += dgrep.16.dt


dt_tests += tseq.1.dt
dt_tests += tseq.2.dt
dt_tests += tseq.3.dt
dt_tests += tseq.4.dt
dt_tests += tseq.5.dt
dt_tests += tseq.6.dt
dt_tests += tseq.7.dt
dt_tests += tseq.8.dt
dt_tests += tseq.9.dt
dt_tests += tseq.10.dt
dt_tests += tseq.11.dt
dt_tests += tseq.12.dt
dt_tests += tseq.13.dt
dt_tests += tseq.14.dt
dt_tests += tseq.15.dt
dt_tests += tseq.16.dt
dt_tests += tseq.17.dt
dt_tests += tseq.18.dt
dt_tests += tseq.19.dt

dt_tests += tdiff.1.dt
dt_tests += tdiff.2.dt
dt_tests += tdiff.3.dt

dt_tests += tadd.1.dt
dt_tests += tadd.2.dt
dt_tests += tadd.3.dt
dt_tests += tadd.4.dt
dt_tests += tadd.5.dt

dt_tests += tgrep.1.dt
dt_tests += tgrep.2.dt
dt_tests += tgrep.3.dt

dt_tests += ttest.1.dt
dt_tests += ttest.2.dt
dt_tests += ttest.3.dt
dt_tests += ttest.4.dt
dt_tests += ttest.5.dt
dt_tests += ttest.6.dt
dt_tests += ttest.7.dt

dt_tests += tconv.1.dt
dt_tests += tconv.2.dt
dt_tests += tconv.3.dt
dt_tests += tconv.4.dt
dt_tests += tconv.5.dt
dt_tests += tconv.6.dt

dt_tests += dtconv.1.dt
dt_tests += dtconv.2.dt
dt_tests += dtconv.3.dt
dt_tests += dtconv.4.dt
dt_tests += dtconv.5.dt
dt_tests += dtconv.6.dt
dt_tests += dtconv.7.dt
dt_tests += dtconv.8.dt
dt_tests += dtconv.9.dt
dt_tests += dtconv.10.dt
dt_tests += dtconv.11.dt
dt_tests += dtconv.12.dt
dt_tests += dtconv.13.dt
dt_tests += dtconv.14.dt
dt_tests += dtconv.15.dt
dt_tests += dtconv.16.dt
dt_tests += dtconv.17.dt
dt_tests += dtconv.18.dt
dt_tests += dtconv.19.dt
dt_tests += dtconv.20.dt
dt_tests += dtconv.21.dt
dt_tests += dtconv.22.dt
dt_tests += dtconv.23.dt
dt_tests += dtconv.24.dt
dt_tests += dtconv.25.dt
dt_tests += dtconv.26.dt
dt_tests += dtconv.27.dt
dt_tests += dtconv.28.dt
dt_tests += dtconv.29.dt
dt_tests += dtconv.30.dt
dt_tests += dtconv.31.dt
dt_tests += dtconv.32.dt
dt_tests += dtconv.33.dt
dt_tests += dtconv.34.dt
dt_tests += dtconv.35.dt

dt_tests += dtseq.1.dt
dt_tests += dtseq.2.dt

dt_tests += dtadd.1.dt
dt_tests += dtadd.2.dt
dt_tests += dtadd.3.dt
dt_tests += dtadd.4.dt
dt_tests += dtadd.5.dt
dt_tests += dtadd.6.dt
dt_tests += dtadd.7.dt
dt_tests += dtadd.8.dt
dt_tests += dtadd.9.dt
dt_tests += dtadd.10.dt
dt_tests += dtadd.11.dt
dt_tests += dtadd.12.dt
dt_tests += dtadd.13.dt
dt_tests += dtadd.14.dt

dt_tests += dtgrep.1.dt
dt_tests += dtgrep.2.dt
dt_tests += dtgrep.3.dt
dt_tests += dtgrep.4.dt
dt_tests += dtgrep.5.dt
dt_tests += dtgrep.6.dt
dt_tests += dtgrep.7.dt
dt_tests += dtgrep.8.dt

dt_tests += dttest.1.dt
dt_tests += dttest.2.dt
dt_tests += dttest.3.dt
dt_tests += dttest.4.dt
dt_tests += dttest.5.dt
dt_tests += dttest.6.dt
dt_tests += dttest.7.dt
dt_tests += dttest.8.dt
dt_tests += dttest.9.dt
dt_tests += dttest.10.dt

dt_tests += dtdiff.1.dt
dt_tests += dtdiff.2.dt
dt_tests += dtdiff.3.dt
dt_tests += dtdiff.4.dt
dt_tests += dtdiff.5.dt
dt_tests += dtdiff.6.dt
dt_tests += dtdiff.7.dt
dt_tests += dtdiff.8.dt
dt_tests += dtdiff.9.dt
dt_tests += dtdiff.10.dt
dt_tests += dtdiff.11.dt
dt_tests += dtdiff.12.dt

dt_tests += strptime.1.dt
dt_tests += strptime.2.dt

check_PROGRAMS += struct-1
check_PROGRAMS += struct-2
check_PROGRAMS += struct-3
check_PROGRAMS += struct-4
check_PROGRAMS += struct-5
check_PROGRAMS += struct-6
check_PROGRAMS += struct-7
check_PROGRAMS += struct-8
check_PROGRAMS += struct-9
check_PROGRAMS += enum-1
check_PROGRAMS += dtcore-strp
check_PROGRAMS += dtcore-add

bin_tests += struct-1
bin_tests += struct-2
bin_tests += struct-3
bin_tests += struct-4
bin_tests += struct-5
bin_tests += struct-6
bin_tests += struct-7
bin_tests += struct-8
bin_tests += struct-9
bin_tests += enum-1
bin_tests += dtcore-strp
bin_tests += dtcore-add

## Makefile.am ends here
