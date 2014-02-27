## bootstrapping rules, made for inclusion in your own Makefiles
yuck.m4i: yuck.yuck
	$(MAKE) $(AM_MAKEFLAGS) yuck-bootstrap
	$(AM_V_GEN) $(builddir)/yuck-bootstrap$(EXEEXT) $(srcdir)/yuck.yuck > $@ \
		|| { rm -f -- $@; false; }

yuck.yucc: yuck.m4i yuck.m4 yuck-coru.h.m4 yuck-coru.c.m4
	$(AM_V_GEN) $(M4) $(srcdir)/yuck.m4 yuck.m4i \
		$(srcdir)/yuck-coru.h.m4 $(srcdir)/yuck-coru.c.m4 | \
		tr '\002\003\016\017' '[]()' > $@ \
		|| { rm -f -- $@; false; }
