# This GNUmakefile is used only if you run GNU Make.

# If the user runs GNU make but has not yet run ./configure,
# give them a diagnostic.
_gl-Makefile := $(wildcard [M]akefile)
ifneq ($(_gl-Makefile),)

include Makefile

# update the included makefile snippet which sets VERSION variables
version.mk: FORCE
	$(top_srcdir)/git-version-gen $(top_srcdir) $@

else

.DEFAULT_GOAL := abort-due-to-no-makefile
$(MAKECMDGOALS): abort-due-to-no-makefile

abort-due-to-no-makefile:
	@echo There seems to be no Makefile in this directory.   1>&2
	@echo "You must run ./configure before running 'make'." 1>&2
	exit 1

endif

.PHONY: FORCE
