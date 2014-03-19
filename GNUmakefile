# This GNUmakefile is used only if you run GNU Make.

# If the user runs GNU make but has not yet run ./configure,
# give them a diagnostic.
_gl-Makefile := $(wildcard [M]akefile)
ifneq ($(_gl-Makefile),)

_dist-target_p ?= $(filter-out %clean, $(filter dist%,$(MAKECMDGOALS)))

include Makefile

# update the included makefile snippet which sets VERSION variables
version.mk: .version version.mk.in FORCE
	-$(AM_V_GEN) \
	if test -w $< -a "$(MAKECMDGOALS)" != "am--refresh"; then \
		$(MAKE) -C "$(top_builddir)/build-aux"; \
		PATH="$(top_builddir)/build-aux:$${PATH}" \
			yuck scmver --ignore-noscm -o $@ --reference $^; \
		if test $$? -eq 3 -a -n "$(_dist-target_p)"; then \
			exec $(MAKE) $(MAKECMDGOALS); \
		fi; \
	fi

else

.DEFAULT_GOAL := abort-due-to-no-makefile
$(MAKECMDGOALS): abort-due-to-no-makefile

abort-due-to-no-makefile:
	@echo There seems to be no Makefile in this directory.   1>&2
	@echo "You must run ./configure before running 'make'." 1>&2
	exit 1

endif

.PHONY: FORCE
