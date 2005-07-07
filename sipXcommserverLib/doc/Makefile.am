# NOTE: Type make 'doc' to build documentation, will not build
# as part of 'make all' for not other reason then it takes a long
# time. This may change.

EXTRA_DIST = \
  Doxyfile.in \
  header.html \
  footer.html \
  credential.xml \
  permission.xml \
  INSTALL.ssl

# 
# If Doxygen is enabled
if DOC

doc: html/index.html

else

# We repeat the doc target in both the "if" and "else" clauses of the
# conditional, because some make programs get upset if no target
# exists.

doc:

endif


html/index.html: Doxyfile
	"@DOXYGEN@"
