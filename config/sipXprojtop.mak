## Makefile components common to the top level of any sipX project

# automake doesn't like this inside the conditional
.PHONY : doc
# 
# If Doxygen is enabled
#
if DOC
doc_SUBDIRS = doc

doc :
	cd doc && $(MAKE) $(AM_MAKEFLAGS) all

endif

