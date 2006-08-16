include config/top.am

RECURSE += \
	check \
	rpmclean \
	clean \
	install \
	uninstall \
	dist \
	rpm \
	distclean

CHECK = check


.PHONY: all dist
dist all:
	@echo "Target '$@' is not valid. Type 'make build' to build and install."

.PHONY: build
build:
	$(MAKE) recurse TARGETS="all $(CHECK) install"

rpm:
	$(MAKE) recurse TARGETS="rpm install-rpms"
