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

## Makefile.am ends here
