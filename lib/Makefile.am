all: default

STANDARD_MAKE_TARGETS = \
	cgicc \
	pcre \
	java-sun \
	java-ibm \
	xerces-c

STANDARD_AUTOCONF_TARGETS = \
	mod_cplusplus

CUSTOM_TARGETS = \
	cgicc \
	pcre \
	java-sun \
	mod_cplusplus

.PHONY: default
default :
	@echo -e "\n\nTo build any dependency cd into that directory and type make, or follow \n\
	instructions in respective directory's README.\n
	 You can also review the makefile for your distro and build all known custom dependencies.\n
	  Example:\n
	     make rhe3 \n
	\n\n"

## These targets are convience targets for your distro and version not meant to be last word on 
## what are required custom dependencies.

.PHONY: fc5
fc5 :	xerces-c cgicc pcre java-sun mod_cplusplus

.PHONY: fc5
fc4 : fc5 ruby ruby-gems ruby-postgres ruby-rails

.PHONY: rhe4
centos4 rhe4 : fc4 w3c-libwww

## NO OTHER TARGETS SHOULD EXISTS.  Each directory should be build-able under one of these 
## two methods.

.PHONY: $(STANDARD_AUTOCONF_TARGETS)
$(STANDARD_AUTOCONF_TARGETS) :
	cd $@; \
	autoreconf -if; \
	./configure; \
	make rpm

.PHONY $(STANDARD_MAKE_TARGETS)
$(STANDARD_MAKE_TARGETS) :
	$(MAKE) -c $@
