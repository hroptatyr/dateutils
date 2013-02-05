# Help the Developers and yourself. Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG = C
LC_ALL = C

libdir = $(abs_top_srcdir)/lib

AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_CPPFLAGS += -DTEST -I$(libdir) -I$(abs_top_builddir)/lib

EXTRA_DIST =
EXTRA_DIST += $(dt_tests)
EXTRA_DIST += $(batch_tests)
EXTRA_DIST += $(BUILT_SOURCES)
TESTS = $(bin_tests) $(dt_tests) $(batch_tests)
TESTS_ENVIRONMENT =
TEST_EXTENSIONS =
BUILT_SOURCES =
dt_tests =
bin_tests =
batch_tests =
check_PROGRAMS =
CLEANFILES = $(check_PROGRAMS)

TEST_EXTENSIONS += .dt
check_PROGRAMS += dt-test
DT_LOG_COMPILER = $(builddir)/dt-test
AM_DT_LOG_FLAGS = --builddir $(top_builddir)/src
EXTRA_DIST += $(srcdir)/dt-test.sh

TESTS_ENVIRONMENT += GPERF=$(GPERF)
TESTS_ENVIRONMENT += GDATE=$(GDATE)
TESTS_ENVIRONMENT += have_gdate="$(have_gdate)"

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
dt_tests += dconv.31.dt
dt_tests += dconv.32.dt
dt_tests += dconv.33.dt
dt_tests += dconv.34.dt
dt_tests += dconv.35.dt
dt_tests += dconv.36.dt
dt_tests += dconv.37.dt
dt_tests += dconv.38.dt
dt_tests += dconv.39.dt
dt_tests += dconv.40.dt
dt_tests += dconv.41.dt
dt_tests += dconv.42.dt
dt_tests += dconv.43.dt
dt_tests += dconv.44.dt
dt_tests += dconv.45.dt
dt_tests += dconv.46.dt
dt_tests += dconv.47.dt
dt_tests += dconv.48.dt
dt_tests += dconv.49.dt
dt_tests += dconv.50.dt
dt_tests += dconv.51.dt
dt_tests += dconv.52.dt
dt_tests += dconv.53.dt
dt_tests += dconv.54.dt
dt_tests += dconv.55.dt
dt_tests += dconv.56.dt
dt_tests += dconv.57.dt
dt_tests += dconv.58.dt
dt_tests += dconv.59.dt
dt_tests += dconv.60.dt
dt_tests += dconv.61.dt
dt_tests += dconv.62.dt
dt_tests += dconv.63.dt
dt_tests += dconv.64.dt
dt_tests += dconv.65.dt
dt_tests += dconv.66.dt
dt_tests += dconv.67.dt
dt_tests += dconv.68.dt
dt_tests += dconv.69.dt
dt_tests += dconv.70.dt
dt_tests += dconv.71.dt
dt_tests += dconv.72.dt
dt_tests += dconv.73.dt
dt_tests += dconv.74.dt
dt_tests += dconv.75.dt
dt_tests += dconv.76.dt
dt_tests += dconv.77.dt
dt_tests += dconv.78.dt
dt_tests += dconv.79.dt
dt_tests += dconv.80.dt
dt_tests += dconv.81.dt
dt_tests += dconv.82.dt
dt_tests += dconv.83.dt
dt_tests += dconv.84.dt
dt_tests += dconv.85.dt
dt_tests += dconv.86.dt
dt_tests += dconv.87.dt
dt_tests += dconv.88.dt
dt_tests += dconv.89.dt
dt_tests += dconv.90.dt
dt_tests += dconv.91.dt
dt_tests += dconv.92.dt
dt_tests += dconv.93.dt
dt_tests += dconv.94.dt
dt_tests += dconv.95.dt
dt_tests += dconv.96.dt

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
dt_tests += dadd.24.dt
dt_tests += dadd.25.dt
dt_tests += dadd.26.dt
dt_tests += dadd.27.dt
dt_tests += dadd.28.dt
dt_tests += dadd.29.dt
dt_tests += dadd.30.dt
dt_tests += dadd.31.dt
dt_tests += dadd.32.dt
dt_tests += dadd.33.dt
dt_tests += dadd.34.dt
dt_tests += dadd.35.dt
dt_tests += dadd.36.dt
dt_tests += dadd.37.dt
dt_tests += dadd.38.dt
dt_tests += dadd.39.dt
dt_tests += dadd.40.dt
dt_tests += dadd.41.dt
dt_tests += dadd.42.dt
dt_tests += dadd.43.dt
dt_tests += dadd.44.dt
dt_tests += dadd.45.dt
dt_tests += dadd.46.dt
dt_tests += dadd.47.dt
dt_tests += dadd.48.dt
dt_tests += dadd.49.dt
dt_tests += dadd.50.dt
dt_tests += dadd.51.dt
dt_tests += dadd.52.dt
dt_tests += dadd.53.dt
dt_tests += dadd.54.dt
dt_tests += dadd.55.dt
dt_tests += dadd.56.dt
dt_tests += dadd.57.dt
dt_tests += dadd.58.dt
dt_tests += dadd.59.dt
dt_tests += dadd.60.dt
dt_tests += dadd.61.dt
dt_tests += dadd.62.dt
dt_tests += dadd.63.dt
dt_tests += dadd.64.dt
dt_tests += dadd.65.dt
dt_tests += dadd.66.dt
dt_tests += dadd.67.dt
dt_tests += dadd.68.dt
dt_tests += dadd.69.dt
dt_tests += dadd.70.dt
dt_tests += dadd.71.dt
dt_tests += dadd.72.dt
dt_tests += dadd.73.dt
dt_tests += dadd.74.dt
dt_tests += dadd.75.dt
dt_tests += dadd.76.dt
dt_tests += dadd.77.dt
dt_tests += dadd.78.dt
dt_tests += dadd.79.dt
dt_tests += dadd.80.dt
dt_tests += dadd.81.dt
dt_tests += dadd.82.dt
dt_tests += dadd.83.dt
dt_tests += dadd.84.dt

dt_tests += dtest.1.dt
dt_tests += dtest.2.dt
dt_tests += dtest.3.dt
dt_tests += dtest.4.dt
dt_tests += dtest.5.dt
dt_tests += dtest.6.dt
dt_tests += dtest.7.dt
dt_tests += dtest.8.dt
dt_tests += dtest.9.dt

dt_tests += ddiff.1.dt
dt_tests += ddiff.2.dt
dt_tests += ddiff.3.dt
dt_tests += ddiff.4.dt
dt_tests += ddiff.5.dt
dt_tests += ddiff.6.dt
dt_tests += ddiff.7.dt
dt_tests += ddiff.8.dt
dt_tests += ddiff.9.dt
dt_tests += ddiff.10.dt
dt_tests += ddiff.11.dt
dt_tests += ddiff.12.dt
dt_tests += ddiff.13.dt
dt_tests += ddiff.14.dt
dt_tests += ddiff.15.dt
dt_tests += ddiff.16.dt

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
dt_tests += dgrep.17.dt
dt_tests += dgrep.18.dt
dt_tests += dgrep.19.dt
dt_tests += dgrep.20.dt

dt_tests += dround.1.dt
dt_tests += dround.2.dt
dt_tests += dround.3.dt
dt_tests += dround.4.dt
dt_tests += dround.5.dt
dt_tests += dround.6.dt
dt_tests += dround.7.dt
dt_tests += dround.8.dt
dt_tests += dround.9.dt
dt_tests += dround.10.dt
dt_tests += dround.11.dt
dt_tests += dround.12.dt
dt_tests += dround.13.dt
dt_tests += dround.14.dt
dt_tests += dround.15.dt
dt_tests += dround.16.dt
dt_tests += dround.17.dt
dt_tests += dround.18.dt
dt_tests += dround.19.dt
dt_tests += dround.20.dt
dt_tests += dround.21.dt
dt_tests += dround.22.dt


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
dt_tests += tdiff.4.dt
dt_tests += tdiff.5.dt
dt_tests += tdiff.6.dt
dt_tests += tdiff.7.dt
dt_tests += tdiff.8.dt
dt_tests += tdiff.9.dt

dt_tests += tadd.1.dt
dt_tests += tadd.2.dt
dt_tests += tadd.3.dt
dt_tests += tadd.4.dt
dt_tests += tadd.5.dt
dt_tests += tadd.6.dt
dt_tests += tadd.7.dt
dt_tests += tadd.8.dt
dt_tests += tadd.9.dt
dt_tests += tadd.10.dt

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

dt_tests += tround.1.dt
dt_tests += tround.2.dt
dt_tests += tround.3.dt
dt_tests += tround.4.dt


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
dt_tests += dtconv.36.dt
dt_tests += dtconv.37.dt
dt_tests += dtconv.38.dt
dt_tests += dtconv.39.dt
dt_tests += dtconv.40.dt
dt_tests += dtconv.41.dt
dt_tests += dtconv.42.dt
dt_tests += dtconv.43.dt
dt_tests += dtconv.44.dt
dt_tests += dtconv.45.dt
dt_tests += dtconv.46.dt
dt_tests += dtconv.47.dt
dt_tests += dtconv.48.dt

dt_tests += convt.ymcw-ymd.dt
dt_tests += convt.ymcw-ywd.dt
dt_tests += convt.ymd-ymcw.dt
dt_tests += convt.ymd-ywd.dt
dt_tests += convt.ywd-ymcw.dt
dt_tests += convt.ywd-ymd.dt

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
dt_tests += dtadd.15.dt
dt_tests += dtadd.16.dt
dt_tests += dtadd.17.dt
dt_tests += dtadd.18.dt
dt_tests += dtadd.19.dt
dt_tests += dtadd.20.dt
dt_tests += dtadd.21.dt
dt_tests += dtadd.22.dt
dt_tests += dtadd.23.dt
dt_tests += dtadd.24.dt
dt_tests += dtadd.25.dt
dt_tests += dtadd.26.dt
dt_tests += dtadd.27.dt
dt_tests += dtadd.28.dt
dt_tests += dtadd.29.dt
if WITH_LEAP_SECONDS
dt_tests += dtadd.30.dt
dt_tests += dtadd.31.dt
endif  ## WITH_LEAP_SECONDS
dt_tests += dtadd.32.dt
dt_tests += dtadd.33.dt
if WITH_LEAP_SECONDS
dt_tests += dtadd.34.dt
dt_tests += dtadd.35.dt
endif  ## WITH_LEAP_SECONDS
dt_tests += dtadd.36.dt
dt_tests += dtadd.37.dt
dt_tests += dtadd.38.dt
dt_tests += dtadd.39.dt
dt_tests += dtadd.40.dt
## multi-sed
dt_tests += dtadd.41.dt
dt_tests += dtadd.42.dt

dt_tests += dtgrep.1.dt
dt_tests += dtgrep.2.dt
dt_tests += dtgrep.3.dt
dt_tests += dtgrep.4.dt
dt_tests += dtgrep.5.dt
dt_tests += dtgrep.6.dt
dt_tests += dtgrep.7.dt
dt_tests += dtgrep.8.dt
dt_tests += dtgrep.9.dt
dt_tests += dtgrep.10.dt
dt_tests += dtgrep.11.dt
dt_tests += dtgrep.12.dt

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
dt_tests += dtdiff.13.dt
dt_tests += dtdiff.14.dt
dt_tests += dtdiff.15.dt
dt_tests += dtdiff.16.dt
dt_tests += dtdiff.17.dt
dt_tests += dtdiff.18.dt
if WITH_LEAP_SECONDS
dt_tests += dtdiff.19.dt
dt_tests += dtdiff.20.dt
dt_tests += dtdiff.21.dt
dt_tests += dtdiff.22.dt
endif  ## WITH_LEAP_SECONDS
dt_tests += dtdiff.23.dt
dt_tests += dtdiff.24.dt
dt_tests += dtdiff.25.dt
dt_tests += dtdiff.26.dt

dt_tests += dtround.1.dt
dt_tests += dtround.2.dt
dt_tests += dtround.3.dt
dt_tests += dtround.4.dt
dt_tests += dtround.5.dt
dt_tests += dtround.6.dt
dt_tests += dtround.7.dt
dt_tests += dtround.8.dt
dt_tests += dtround.9.dt
dt_tests += dtround.10.dt
dt_tests += dtround.11.dt
dt_tests += dtround.12.dt
dt_tests += dtround.13.dt
dt_tests += dtround.14.dt

dt_tests += strptime.1.dt
dt_tests += strptime.2.dt

## we should check if the zones in these tests actually exist
## also failing tests here should be soft-fails because that could
## easily be an outdated zoneinfo file
dt_tests += dtz.1.dt
dt_tests += dtz.2.dt

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
check_PROGRAMS += dtcore-conv
check_PROGRAMS += dtcore-add
check_PROGRAMS += time-core-add
check_PROGRAMS += basic_ymd_get_wday
check_PROGRAMS += basic_get_jan01_wday
check_PROGRAMS += basic_md_get_yday
check_PROGRAMS += basic_get_dom_wday

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
bin_tests += dtcore-conv
bin_tests += dtcore-add
bin_tests += time-core-add
bin_tests += basic_ymd_get_wday
bin_tests += basic_get_jan01_wday
bin_tests += basic_get_dom_wday
bin_tests += basic_md_get_yday

## batch checks
batch_tests += dseq-cnt.1.sh
if HAVE_GDATE
TESTS_ENVIRONMENT += have_gdate_2039="$(have_gdate_2039)"
batch_tests += dseq-cnt.2.sh
batch_tests += dseq-cnt.3.sh
batch_tests += dconv-batch.1.sh
batch_tests += dconv-batch.2.sh
endif  HAVE_GDATE


check_PROGRAMS += dt-test
BUILT_SOURCES += dt-test-clo.c dt-test-clo.h
EXTRA_DIST += dt-test.sh

## ggo rule
%.c %.h: %.ggo
	$(AM_V_GEN) gengetopt -F $* -i $<

clean-local:
	-rm -rf *.tmpd

## Makefile.am ends here
