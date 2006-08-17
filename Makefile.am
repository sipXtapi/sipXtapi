include config/top.am


RECURSE += \
	check \
	rpmclean \
	clean \
	install \
	uninstall \
	distclean

CHECK = check


.PHONY: all
all:
	@echo "Target '$@' is not valid. Type 'make build' to build and install."

.PHONY: build
build:
	$(MAKE) recurse TARGETS="all $(CHECK) install"

rpm:
	$(MAKE) recurse TARGETS="rpm install-rpms"

# causes duplicate target error!
.PHONY:
dists:
	$(MAKE) recurse TARGETS="dist"
