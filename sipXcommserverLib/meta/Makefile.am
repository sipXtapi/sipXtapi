include $(top_builddir)/config/sipXcommon.mak

schemas = \
	urlmap.xsd \
	authrules.xsd 

EXTRA_DATA = $(schemas)

EXTRA_DIST = \
	$(EXTRA_DATA)

install-data-hook : install-schema

.PHONY: install-schema
install-schema: $(foreach xsd,$(schemas),$(DESTDIR)@SIPX_SCHEMADIR@/$(xsd))

$(foreach xsd,$(schemas),$(DESTDIR)@SIPX_SCHEMADIR@/$(xsd)): $(DESTDIR)@SIPX_SCHEMADIR@/% : $(srcdir)/%
	$(INSTALL) -D -m 644 $< $@
