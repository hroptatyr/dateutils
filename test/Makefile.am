# Help the Developers and yourself. Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG = C
LC_ALL = C

EXTRA_DIST = $(TESTS)
EXTRA_DIST += $(DT_LOG_COMPILER)
TESTS =

TEST_EXTENSIONS = .dt
DT_LOG_COMPILER = $(srcdir)/dt-test.sh
AM_DT_LOG_FLAGS = --builddir $(top_builddir)/src --hash sha1sum
LOG_COMPILER = echo

TESTS += dseq.1.dt
TESTS += dseq.2.dt
TESTS += dseq.3.dt
TESTS += dseq.4.dt
TESTS += dseq.5.dt
TESTS += dseq.6.dt
TESTS += dseq.7.dt
TESTS += dseq.8.dt
TESTS += dseq.9.dt
TESTS += dseq.10.dt
TESTS += dseq.11.dt
TESTS += dseq.12.dt
TESTS += dseq.13.dt
TESTS += dseq.14.dt
TESTS += dseq.15.dt
TESTS += dseq.16.dt
TESTS += dseq.17.dt
TESTS += dseq.18.dt

TESTS += dcal.1.dt
TESTS += dcal.2.dt
TESTS += dcal.3.dt
TESTS += dcal.4.dt
TESTS += dcal.5.dt
TESTS += dcal.6.dt
TESTS += dcal.7.dt
TESTS += dcal.8.dt

TESTS += dadd.1.dt
TESTS += dadd.2.dt
TESTS += dadd.3.dt
TESTS += dadd.4.dt
TESTS += dadd.5.dt
TESTS += dadd.6.dt

TESTS += dtest.1.dt
TESTS += dtest.2.dt
TESTS += dtest.3.dt
TESTS += dtest.4.dt
TESTS += dtest.5.dt

TESTS += ddiff.1.dt
TESTS += ddiff.2.dt
TESTS += ddiff.3.dt
TESTS += ddiff.4.dt
TESTS += ddiff.5.dt

TESTS += tseq.1.dt
TESTS += tseq.2.dt
TESTS += tseq.3.dt
TESTS += tseq.4.dt
TESTS += tseq.5.dt
TESTS += tseq.6.dt
TESTS += tseq.7.dt
TESTS += tseq.8.dt

TESTS += tdiff.1.dt
TESTS += tdiff.2.dt
TESTS += tdiff.3.dt

TESTS += tadd.1.dt
TESTS += tadd.2.dt
TESTS += tadd.3.dt
TESTS += tadd.4.dt
TESTS += tadd.5.dt

## Makefile.am ends here
