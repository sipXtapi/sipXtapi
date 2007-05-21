# Automated build makefile.
# This makefile does everything needed to build sipXtapi, up through sipXmediaAdapterLib.
# It was built to enable automated builds, but ordinary users can use it.
# Users can pass in their own additional autoreconf and configure options by 
# putting them in the environment variables AUTORECONFFLAGS and CONFIGFLAGS respectively.

# Guidelines for changes, with respect to the automated build.
# If changes are made to this, just make sure that the following capabilities remain:
# *  'make' successfully builds all libraries through sipXmediaAdapterLib from 
#    a fresh sandbox.
# *  'make distclean' removes all (or nearly all) generated files, such that a 
#    freshly checked out tree that has been built, then cleaned with 
#    'make distclean' will look like a freshly checked out tree (or nearly so)
#    when svn stat is called.
# *  'make check' makes all unit tests through sipXmediaAdapterLib, and executes them.

AUTORECONF=autoreconf -if
PROJECTS=sipXportLib \
         sipXsdpLib \
         sipXtackLib \
         sipXmediaLib \
         sipXmediaAdapterLib 
#         sipXcallLib 
#         bbridge \
#         sipXcommserverLib \
#         sipXproxy \
#         sipXregistry \
#         sipXpublisher \
#         sipXvxml 
#         sipXpbx \
#         sipXconfig \
#         sipXphone 
         #sipXtest



libs : 
	$(foreach PROJ, $(PROJECTS), $(MAKE) -f Makefile.gnu $(PROJ)/src/$(patsubst %Lib,lib%.la,$(PROJ)); )
makefiles Makefiles : 
	$(foreach PROJ, $(PROJECTS), $(MAKE) -f Makefile.gnu $(PROJ)/Makefile; )
configure : 
	$(foreach PROJ, $(PROJECTS), $(MAKE) -f Makefile.gnu $(PROJ)/configure; )
check : 
	$(foreach PROJ, $(PROJECTS), \
	  $(warning "$(PROJ): Building and running unit tests") \
	  $(if $(wildcard $(PROJ)/Makefile), \
	    $(MAKE) -C $(PROJ) $(@); , \
	    $(warning "No Library for $(PROJ)! Cannot make $(PROJ) unit tests.")))



$(addsuffix /configure,$(PROJECTS)) : #$(addsuffix /configure.ac,$(PROJECTS)) $(addsuffix /Makefile.am,$(PROJECTS))
	$(warning "$(@D): Creating configure script with autoreconf")
	cd $(@D); autoreconf --force --install $(AUTORECONFFLAGS); cd ..



$(addsuffix /Makefile,$(PROJECTS)) : #$(addsuffix /configure,$(PROJECTS))
	$(MAKE) -f Makefile.gnu $(@D)/configure
	$(warning "$(@D): creating Makefiles with configure")
	cd $(@D); ./configure --disable-sipviewer --enable-local-audio $(CONFIGFLAGS); cd ..


LIB_LA_FILES = $(foreach PROJECT, $(PROJECTS), $(PROJECT)/src/$(patsubst %Lib,lib%.la,$(PROJECT)) )
$(LIB_LA_FILES) : 
	$(MAKE) -f Makefile.gnu $(dir $(@D))Makefile
	$(warning "$(patsubst %/,%,$(dir $(@D))): Building library")
	cd $(dir $(@D)); make; cd ..





.PHONY: libs makefiles Makefiles clean confclean

clean: 
	$(foreach PROJECT, $(PROJECTS), cd $(PROJECT); make clean; cd ..; )

distclean: clean
	$(foreach PROJECT, $(PROJECTS), cd $(PROJECT); make distclean; cd ..; )
	rm -f $(shell find . -name Makefile.in)
	$(foreach PROJECT, $(PROJECTS), cd $(PROJECT); rm -Rf config.h.in aclocal.m4 configure autom4te.cache; cd ..; )
	$(foreach PROJECT, $(PROJECTS), cd $(PROJECT); rm -f config.log config.h config.status stamp-h1 BUILDSTAMP libtool; cd ..; )
	$(foreach PROJECT, $(PROJECTS), cd $(PROJECT)/config; rm -f depcomp missing config.guess ltmain.sh config.sub install-sh; cd ../..; )

