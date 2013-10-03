# Help the Developers and yourself. Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
include $(top_builddir)/version.mk

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

TEST_EXTENSIONS += .clit
TESTS_ENVIRONMENT += root=$(top_srcdir)
CLIT_LOG_COMPILER = $(builddir)/clitoris
AM_CLIT_LOG_FLAGS = -v --builddir $(top_builddir)/src --timeout 10

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
dt_tests += dconv.97.dt
dt_tests += dconv.98.dt
dt_tests += dconv.99.dt
dt_tests += dconv.100.dt
dt_tests += dconv.101.dt
dt_tests += dconv.102.dt

dt_tests += dadd.001.clit
dt_tests += dadd.002.clit
dt_tests += dadd.003.clit
dt_tests += dadd.004.clit
dt_tests += dadd.005.clit
dt_tests += dadd.006.clit
dt_tests += dadd.007.clit
dt_tests += dadd.008.clit
dt_tests += dadd.009.clit
dt_tests += dadd.010.clit
dt_tests += dadd.011.clit
dt_tests += dadd.012.clit
dt_tests += dadd.013.clit
dt_tests += dadd.014.clit
dt_tests += dadd.015.clit
dt_tests += dadd.016.clit
dt_tests += dadd.017.clit
dt_tests += dadd.018.clit
dt_tests += dadd.019.clit
dt_tests += dadd.020.clit
dt_tests += dadd.021.clit
dt_tests += dadd.022.clit
dt_tests += dadd.023.clit
dt_tests += dadd.024.clit
dt_tests += dadd.025.clit
dt_tests += dadd.026.clit
dt_tests += dadd.027.clit
dt_tests += dadd.028.clit
dt_tests += dadd.029.clit
dt_tests += dadd.030.clit
dt_tests += dadd.031.clit
dt_tests += dadd.032.clit
dt_tests += dadd.033.clit
dt_tests += dadd.034.clit
dt_tests += dadd.035.clit
dt_tests += dadd.036.clit
dt_tests += dadd.037.clit
dt_tests += dadd.038.clit
dt_tests += dadd.039.clit
dt_tests += dadd.040.clit
dt_tests += dadd.041.clit
dt_tests += dadd.042.clit
dt_tests += dadd.043.clit
dt_tests += dadd.044.clit
dt_tests += dadd.045.clit
dt_tests += dadd.046.clit
dt_tests += dadd.047.clit
dt_tests += dadd.048.clit
dt_tests += dadd.049.clit
dt_tests += dadd.050.clit
dt_tests += dadd.051.clit
dt_tests += dadd.052.clit
dt_tests += dadd.053.clit
dt_tests += dadd.054.clit
dt_tests += dadd.055.clit
dt_tests += dadd.056.clit
dt_tests += dadd.057.clit
dt_tests += dadd.058.clit
dt_tests += dadd.059.clit
dt_tests += dadd.060.clit
dt_tests += dadd.061.clit
dt_tests += dadd.062.clit
dt_tests += dadd.063.clit
dt_tests += dadd.064.clit
dt_tests += dadd.065.clit
dt_tests += dadd.066.clit
dt_tests += dadd.067.clit
dt_tests += dadd.068.clit
dt_tests += dadd.069.clit
dt_tests += dadd.070.clit
dt_tests += dadd.071.clit
dt_tests += dadd.072.clit
dt_tests += dadd.073.clit
dt_tests += dadd.074.clit
dt_tests += dadd.075.clit
dt_tests += dadd.076.clit
dt_tests += dadd.077.clit
dt_tests += dadd.078.clit
dt_tests += dadd.079.clit
dt_tests += dadd.080.clit
dt_tests += dadd.081.clit
dt_tests += dadd.082.clit
dt_tests += dadd.083.clit
dt_tests += dadd.084.clit
dt_tests += dadd.085.clit
dt_tests += dadd.086.clit

dt_tests += dtest.1.dt
dt_tests += dtest.2.dt
dt_tests += dtest.3.dt
dt_tests += dtest.4.dt
dt_tests += dtest.5.dt
dt_tests += dtest.6.dt
dt_tests += dtest.7.dt
dt_tests += dtest.8.dt
dt_tests += dtest.9.dt

dt_tests += ddiff.001.clit
dt_tests += ddiff.002.clit
dt_tests += ddiff.003.clit
dt_tests += ddiff.004.clit
dt_tests += ddiff.005.clit
dt_tests += ddiff.006.clit
dt_tests += ddiff.007.clit
dt_tests += ddiff.008.clit
dt_tests += ddiff.009.clit
dt_tests += ddiff.010.clit
dt_tests += ddiff.011.clit
dt_tests += ddiff.012.clit
dt_tests += ddiff.013.clit
dt_tests += ddiff.014.clit
dt_tests += ddiff.015.clit
dt_tests += ddiff.016.clit

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

dt_tests += dround.001.clit
dt_tests += dround.002.clit
dt_tests += dround.003.clit
dt_tests += dround.004.clit
dt_tests += dround.005.clit
dt_tests += dround.006.clit
dt_tests += dround.007.clit
dt_tests += dround.008.clit
dt_tests += dround.009.clit
dt_tests += dround.010.clit
dt_tests += dround.011.clit
dt_tests += dround.012.clit
dt_tests += dround.013.clit
dt_tests += dround.014.clit
dt_tests += dround.015.clit
dt_tests += dround.016.clit
dt_tests += dround.017.clit
dt_tests += dround.018.clit
dt_tests += dround.019.clit
dt_tests += dround.020.clit
dt_tests += dround.021.clit
dt_tests += dround.022.clit
dt_tests += dround.023.clit
dt_tests += dround.024.clit


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

dt_tests += tdiff.001.clit
dt_tests += tdiff.002.clit
dt_tests += tdiff.003.clit
dt_tests += tdiff.004.clit
dt_tests += tdiff.005.clit
dt_tests += tdiff.006.clit
dt_tests += tdiff.007.clit
dt_tests += tdiff.008.clit
dt_tests += tdiff.009.clit

dt_tests += tadd.001.clit
dt_tests += tadd.002.clit
dt_tests += tadd.003.clit
dt_tests += tadd.004.clit
dt_tests += tadd.005.clit
dt_tests += tadd.006.clit
dt_tests += tadd.007.clit
dt_tests += tadd.008.clit
dt_tests += tadd.009.clit
dt_tests += tadd.010.clit

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

dt_tests += tround.001.clit
dt_tests += tround.002.clit
dt_tests += tround.003.clit
dt_tests += tround.004.clit


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

dt_tests += dtadd.001.clit
dt_tests += dtadd.002.clit
dt_tests += dtadd.003.clit
dt_tests += dtadd.004.clit
dt_tests += dtadd.005.clit
dt_tests += dtadd.006.clit
dt_tests += dtadd.007.clit
dt_tests += dtadd.008.clit
dt_tests += dtadd.009.clit
dt_tests += dtadd.010.clit
dt_tests += dtadd.011.clit
dt_tests += dtadd.012.clit
dt_tests += dtadd.013.clit
dt_tests += dtadd.014.clit
dt_tests += dtadd.015.clit
dt_tests += dtadd.016.clit
dt_tests += dtadd.017.clit
dt_tests += dtadd.018.clit
dt_tests += dtadd.019.clit
dt_tests += dtadd.020.clit
dt_tests += dtadd.021.clit
dt_tests += dtadd.022.clit
dt_tests += dtadd.023.clit
dt_tests += dtadd.024.clit
dt_tests += dtadd.025.clit
dt_tests += dtadd.026.clit
dt_tests += dtadd.027.clit
dt_tests += dtadd.028.clit
dt_tests += dtadd.029.clit
if WITH_LEAP_SECONDS
dt_tests += dtadd.030.clit
dt_tests += dtadd.031.clit
endif  ## WITH_LEAP_SECONDS
dt_tests += dtadd.032.clit
dt_tests += dtadd.033.clit
if WITH_LEAP_SECONDS
dt_tests += dtadd.034.clit
dt_tests += dtadd.035.clit
endif  ## WITH_LEAP_SECONDS
dt_tests += dtadd.036.clit
dt_tests += dtadd.037.clit
dt_tests += dtadd.038.clit
dt_tests += dtadd.039.clit
dt_tests += dtadd.040.clit
## multi-sed
dt_tests += dtadd.041.clit
dt_tests += dtadd.042.clit

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

dt_tests += dtdiff.001.clit
dt_tests += dtdiff.002.clit
dt_tests += dtdiff.003.clit
dt_tests += dtdiff.004.clit
dt_tests += dtdiff.005.clit
dt_tests += dtdiff.006.clit
dt_tests += dtdiff.007.clit
dt_tests += dtdiff.008.clit
dt_tests += dtdiff.009.clit
dt_tests += dtdiff.010.clit
dt_tests += dtdiff.011.clit
dt_tests += dtdiff.012.clit
dt_tests += dtdiff.013.clit
dt_tests += dtdiff.014.clit
dt_tests += dtdiff.015.clit
dt_tests += dtdiff.016.clit
dt_tests += dtdiff.017.clit
dt_tests += dtdiff.018.clit
if WITH_LEAP_SECONDS
dt_tests += dtdiff.019.clit
dt_tests += dtdiff.020.clit
dt_tests += dtdiff.021.clit
dt_tests += dtdiff.022.clit
endif  ## WITH_LEAP_SECONDS
dt_tests += dtdiff.023.clit
dt_tests += dtdiff.024.clit
dt_tests += dtdiff.025.clit
dt_tests += dtdiff.026.clit
dt_tests += dtdiff.027.clit
dt_tests += dtdiff.028.clit
dt_tests += dtdiff.029.clit
dt_tests += dtdiff.030.clit
dt_tests += dtdiff.031.clit
dt_tests += dtdiff.032.clit

dt_tests += dtround.001.clit
dt_tests += dtround.002.clit
dt_tests += dtround.003.clit
dt_tests += dtround.004.clit
dt_tests += dtround.005.clit
dt_tests += dtround.006.clit
dt_tests += dtround.007.clit
dt_tests += dtround.008.clit
dt_tests += dtround.009.clit
dt_tests += dtround.010.clit
dt_tests += dtround.011.clit
dt_tests += dtround.012.clit
dt_tests += dtround.013.clit
dt_tests += dtround.014.clit

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
EXTRA_DIST += dt-test.ggo
BUILT_SOURCES += dt-test.x dt-test.xh
EXTRA_DIST += dt-test.sh

check_PROGRAMS += clitoris
clitoris_CPPFLAGS = $(AM_CPPFLAGS)
clitoris_CPPFLAGS += -D_GNU_SOURCE
clitoris_LDADD = -lutil
EXTRA_DIST += clitoris.ggo
BUILT_SOURCES += clitoris.x clitoris.xh


## ggo rule
%.x %.xh: %.ggo
	$(AM_V_GEN) gengetopt -F $* -c x -H xh -i $<
	@sed -e 's,config.h,version.h,g' \
	     -e 's,_PARSER_VERSION VERSION,_PARSER_VERSION "$(VERSION)",g' \
	     $*.xh > $*.xh-t \
	&& mv $*.xh-t $*.xh

clean-local:
	-rm -rf *.tmpd

## Makefile.am ends here
