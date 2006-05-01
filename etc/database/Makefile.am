
INSTALL=@INSTALL@

include $(top_builddir)/config/sipXcommon.mak

sql_files = \
    schema.sql \
    refer_uri.sql

EXTRA_DIST = \
    $(sql_files)

install-exec-hook : $(foreach file,$(sql_files),$(DESTDIR)/$(sysconfdir)/sipxpbx/cdr/$(file))

$(foreach file,$(sql_files),$(DESTDIR)/$(sysconfdir)/sipxpbx/cdr/$(file)) : $(DESTDIR)/$(sysconfdir)/sipxpbx/cdr/% : %
	$(INSTALL) -D -m 644 $< $@; 

