all: doc

EXTRA_DIST = \
  Doxyfile.in \
  header.html \
  footer.html

doc: html/index.html

html/index.html: Doxyfile
	"@DOXYGEN@"
