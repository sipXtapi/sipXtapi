
# rpm
# rpm clean
# build * (all [check] install-if-lib)
# build target[dependencies] (all [check] install-if-lib)
# dist
# 
# options
#  failfast
#  sudo

RECURSE = \
	check \
	rpmclean \
	clean \
	install \
	uninstall \
	recurse \
	dist

CHECK = check

PROJECTS = $(subdirs)

$(RECURSE):
	@failcom='exit 1'; \
	for f in x $$MAKEFLAGS; do \
	  case $$f in \
	    *=* | --[!k]*);; \
	    *k*) failcom='fail=yes';; \
	  esac; \
	done; \
	target="$(TARGETS)`echo $@ | sed -e s/recurse//`"; \
	list='$(PROJECTS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$target) \
	  || eval $$failcom; \
	done;

# simply list projects in correct build order
.PHONY: list
list:
	@echo $(PROJECTS)

.PHONY: all dist
all:
	@echo "Target is not valid."

.PHONY: build
build:
	$(MAKE) recurse TARGETS="all $(CHECK) install"

# TODO rpm-install should only install if it's a dependency
rpm:
	$(MAKE) recurse TARGETS="rpm rpm-install"
