## Wrapper for building resiprocate as a set of static libraries

all: rutil resip dum

check: resip-test

install: install-rutil install-resip install-dum

@abs_top_srcdir@/resiprocate/build/Makefile.conf:
	@abs_top_srcdir@/resiprocate/configure -y --prefix=@prefix@

.PHONY: rutil
rutil: @abs_top_srcdir@/resiprocate/build/Makefile.conf
	$(MAKE) -C @abs_top_srcdir@/resiprocate rutil

.PHONY: install-rutil
install-rutil:
	$(MAKE) -C @abs_top_srcdir@/resiprocate install-rutil

.PHONY: resip
resip: @abs_top_srcdir@/resiprocate/build/Makefile.conf
	$(MAKE) -C @abs_top_srcdir@/resiprocate resiprocate

.PHONY: resip-test
resip-test:
	$(MAKE) -C @abs_top_srcdir@/resiprocate tests

.PHONY: install-resip
install-resip:
	$(MAKE) -C @abs_top_srcdir@/resiprocate install-resip

.PHONY: dum
dum: @abs_top_srcdir@/resiprocate/build/Makefile.conf
	$(MAKE) -C @abs_top_srcdir@/resiprocate dum 

.PHONY: install-dum
install-dum:
	$(MAKE) -C @abs_top_srcdir@/resiprocate install-dum

clean: clean-resiprocate

.PHONY: clean-resiprocate
clean-resiprocate:
	$(MAKE) -C @abs_top_srcdir@/resiprocate clean
	rm -f @abs_top_srcdir@/resiprocate/build/Makefile.conf


