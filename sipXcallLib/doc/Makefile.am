include $(top_srcdir)/config/sipXcommon.am.inc

EXTRA_DIST = \
  Doxyfile.in \
  header.html \
  footer.html

all: doc

PROJECT = sipXcallLib

doc: $(PROJECT)

$(PROJECT): Doxyfile
	rm -rf $(PROJECT)
	@DOXYGEN@

Doxyfile: $(srcdir)/Doxyfile.in
	$(LocalizeSipXconfig) \
		-e 's,@enable_html_docs\@,@enable_html_docs@,g' \
		-e 's,@enable_latex_docs\@,@enable_latex_docs@,g' \
		-e 's,@enable_dot\@,@enable_dot@,g' \
		-e 's,@srcdir\@,$(srcdir),g' \
		-e 's,@top_srcdir\@,$(top_srcdir),g' \
	$(srcdir)/Doxyfile.in > Doxyfile

install: install-doc

.PHONY: install-doc
install-doc: $(PROJECT)
	rm -rf $(DESTDIR)@docdir@/$(PROJECT)
	@INSTALL@ -d $(DESTDIR)@docdir@/$(PROJECT)
	find $(PROJECT) -type f -print \
	| xargs --replace=% @INSTALL@ -m u=rw,go=r % $(DESTDIR)@docdir@/%

clean-local: clean-doc

.PHONY: clean-doc
clean-doc:
	rm -rf $(PROJECT) Doxyfile

DISTCLEANFILES = Doxyfile
