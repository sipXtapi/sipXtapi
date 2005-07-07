# NOTE: Type make 'doc' to build documentation, will not build
# as part of 'make all' for not other reason then it takes a long
# time. This may change.

EXTRA_DIST = \
  Doxyfile.in \
  header.html \
  footer.html \
  license/jdom-LICENSE.txt \
  license/junit-LICENSE.txt \
  license/MD5-LICENSE

all: doc

doc: html/index.html

html/index.html: Doxyfile
	"@DOXYGEN@"
