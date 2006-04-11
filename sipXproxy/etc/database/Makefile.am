
INSTALL=@INSTALL@

include $(top_builddir)/config/sipXcommon.mak

sql_schemas = \
    schema.sql

EXTRA_DIST = \
    $(sql_schemas)

install-exec-hook : $(foreach file,$(sql_schemas),$(DESTDIR)/$(sysconfdir)/sipxpbx/cdr/$(file))

$(foreach file,$(sql_schemas),$(DESTDIR)/$(sysconfdir)/sipxpbx/cdr/$(file)) : $(DESTDIR)/$(sysconfdir)/sipxpbx/cdr/% : %
	$(INSTALL) -D -m 644 $< $@; 

