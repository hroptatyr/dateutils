# Help the Developers and yourself.  Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG=C
LC_ALL=C

info_TEXINFOS = datetools.texi
info_TEXINFOS +=

TEXI2DVI_TEX = latex
TEXI2DVI = TEX=$(TEXI2DVI_TEX) texi2dvi -c -s

man1_MANS =
man1_MANS += strptime.man
man1_MANS += dcal.man
man1_MANS += dseq.man
man1_MANS += dtest.man

%.man: $(top_builddir)/src/%$(EXEEXT)
	$(HELP2MAN) -o $@ $<
