# A prefix command applied to commands that manipulate the destination
# directories.  Left as null to prevent surprises for people who haven't
# read this Makefile before using it, but often set to "sudo".
SUDO=

# The names of the destination directories.
# These values are the defaults for RPM installation, but can be retargeted.
# See http://www.sipfoundry.org/sipX/doc/filesystem.html.
# Note that changing these values is not completely supported yet --
# some of the installed files would need to be adjusted.
SIPX_BINDIR=/usr/bin
SIPX_CONFDIR=/etc/sipxpbx
SIPX_DBDIR=/var/sipxdata/sipdb
SIPX_LIBDIR=/usr/lib
HTTP_ROOTDIR=/usr/share/www/doc

# The names of all the installed files, derived from the files in
# the install subdirectories.
CONFIG_FILES=$(subst sipx_confdir,$(SIPX_CONFDIR),$(filter-out %~,$(wildcard sipx_confdir/*)))
DOC_FILES=$(subst http_rootdir,$(HTTP_ROOTDIR),$(filter-out %~,$(wildcard http_rootdir/*)))
SIPDB_FILES=$(subst sipx_dbdir,$(SIPX_DBDIR),$(filter-out %~,$(wildcard sipx_dbdir/*)))

CONFIG_FILES_EDITED=$(subst .pre,,$(subst sipx_confdir,$(SIPX_CONFDIR),$(filter-out %~,$(wildcard sipx_confdir/*.pre))))
DOC_FILES_EDITED=$(subst .pre,,$(subst http_rootdir,$(HTTP_ROOTDIR),$(filter-out %~,$(wildcard http_rootdir/*.pre))))
SIPDB_FILES_EDITED=$(subst .pre,,$(subst sipx_dbdir,$(SIPX_DBDIR),$(filter-out %~,$(wildcard sipx_dbdir/*.pre))))

install: debug $(CONFIG_FILES) $(DOC_FILES) $(SIPDB_FILES) $(CONFIG_FILES_EDITED) $(DOC_FILES_EDITED) $(SIBDB_FILES_EDITED) $(SIPX_CONFDIR)/registrar-config.in

debug:
	@echo CONFIG_FILES=$(CONFIG_FILES)
	@echo DOC_FILES=$(DOC_FILES)
	@echo SIPDB_FILES=$(SIPDB_FILES)
	@echo DOC_FILES_EDITED=$(DOC_FILES_EDITED) 
	@echo CONFIG_FILES_EDITED=$(CONFIG_FILES_EDITED)
	@echo SIPDB_FILES_EDITED=$(SIPDB_FILES_EDITED)

$(CONFIG_FILES): $(SIPX_CONFDIR)/%: sipx_confdir/%
	$(SUDO) cp $< $@

$(DOC_FILES): $(HTTP_ROOTDIR)/%: http_rootdir/%
	$(SUDO) cp $< $@

$(SIPDB_FILES): $(SIPX_DBDIR)/%: sipx_dbdir/%
	$(SUDO) cp $< $@

$(SIPX_CONFDIR)/config.defs: $(SIPX_CONFDIR)/config.defs.in
	$(SUDO) sh -c \
		"sed \
			-e 's!@SIPX_BINDIR@!$(SIPX_BINDIR)!g' \
			-e 's!@SIPX_CONFDIR@!$(SIPX_CONFDIR)!g' \
			-e 's!@SIPX_DBDIR@!$(SIPX_DBDIR)!g' \
			-e 's!@SIPX_LIBDIR@!$(SIPX_LIBDIR)!g' \
			-e 's!@HTTP_ROOTDIR@!$(HTTP_ROOTDIR)!g' \
		<$< >$@"

$(CONFIG_FILES_EDITED): %: %.pre $(SIPX_CONFDIR)/config.defs
	. $(SIPX_CONFDIR)/config.defs ;\
	HOSTNAME=`hostname -f` ;\
	$(SUDO) sh -c \
		"sed \
			-e 's!@SIPXCHANGE_DOMAIN_NAME@!$$SIPXCHANGE_DOMAIN_NAME!g' \
			-e 's!@SIPX_BINDIR@!$(SIPX_BINDIR)!g' \
			-e 's!@SIPX_CONFDIR@!$(SIPX_CONFDIR)!g' \
			-e 's!@SIPX_DBDIR@!$(SIPX_DBDIR)!g' \
			-e 's!@SIPX_LIBDIR@!$(SIPX_LIBDIR)!g' \
			-e 's!@HTTP_ROOTDIR@!$(HTTP_ROOTDIR)!g' \
			-e 's!@HOSTNAME@!$$HOSTNAME!g' \
		<$< >$@"

$(DOC_FILES_EDITED): %: %.pre $(SIPX_CONFDIR)/config.defs
	. $(SIPX_CONFDIR)/config.defs ;\
	HOSTNAME=`hostname -f` ;\
	$(SUDO) sh -c \
		"sed \
			-e 's!@SIPXCHANGE_DOMAIN_NAME@!$$SIPXCHANGE_DOMAIN_NAME!g' \
			-e 's!@SIPX_BINDIR@!$(SIPX_BINDIR)!g' \
			-e 's!@SIPX_CONFDIR@!$(SIPX_CONFDIR)!g' \
			-e 's!@SIPX_DBDIR@!$(SIPX_DBDIR)!g' \
			-e 's!@SIPX_LIBDIR@!$(SIPX_LIBDIR)!g' \
			-e 's!@HTTP_ROOTDIR@!$(HTTP_ROOTDIR)!g' \
			-e 's!@HOSTNAME@!$$HOSTNAME!g' \
		<$< >$@"

$(SIPDB_FILES_EDITED): %: %.pre $(SIPX_CONFDIR)/config.defs
	. $(SIPX_CONFDIR)/config.defs ;\
	HOSTNAME=`hostname -f` ;\
	$(SUDO) sh -c \
		"sed \
			-e 's!@SIPXCHANGE_DOMAIN_NAME@!$$SIPXCHANGE_DOMAIN_NAME!g' \
			-e 's!@SIPX_BINDIR@!$(SIPX_BINDIR)!g' \
			-e 's!@SIPX_CONFDIR@!$(SIPX_CONFDIR)!g' \
			-e 's!@SIPX_DBDIR@!$(SIPX_DBDIR)!g' \
			-e 's!@SIPX_LIBDIR@!$(SIPX_LIBDIR)!g' \
			-e 's!@HTTP_ROOTDIR@!$(HTTP_ROOTDIR)!g' \
			-e 's!@HOSTNAME@!$$HOSTNAME!g' \
		<$< >$@"


# The special rule to perform substitutions in index.html.in to create
# index.html.  This rule is done in the destination directory rather than
# in the source directory (1) so multiple installations can be done from
# one source directory, and (2) so if SIPXCHANGE_DOMAIN_NAME in config.defs
# needs to be changed after installation, re-running "make" will correct
# index.html.
#$(HTTP_ROOTDIR)/index.html: $(HTTP_ROOTDIR)/index.html.pre $(SIPX_CONFDIR)/config.defs
#	. $(SIPX_CONFDIR)/config.defs ;\
#	HOSTNAME=`hostname -f` ;\
#	$(SUDO) sh -c \
#		"sed \
#			-e 's!@SIPXCHANGE_DOMAIN_NAME@!$$SIPXCHANGE_DOMAIN_NAME!g' \
#			-e 's!@SIPX_BINDIR@!$(SIPX_BINDIR)!g' \
#			-e 's!@SIPX_CONFDIR@!$(SIPX_CONFDIR)!g' \
#			-e 's!@SIPX_DBDIR@!$(SIPX_DBDIR)!g' \
#			-e 's!@SIPX_LIBDIR@!$(SIPX_LIBDIR)!g' \
#			-e 's!@HTTP_ROOTDIR@!$(HTTP_ROOTDIR)!g' \
#			-e 's!@HOSTNAME@!$$HOSTNAME!g' \
#		<$< >$@"
#
#$(SIPX_CONFDIR)/registrar-config.in: $(SIPX_CONFDIR)/registrar-config.in.pre $(SIPX_CONFDIR)/config.defs
#	. $(SIPX_CONFDIR)/config.defs ;\
#	$(SUDO) sh -c \
#		"sed \
#			-e 's!@SIPXCHANGE_DOMAIN_NAME@!$$SIPXCHANGE_DOMAIN_NAME!g' \
#			-e 's!@SIPX_BINDIR@!$(SIPX_BINDIR)!g' \
#			-e 's!@SIPX_CONFDIR@!$(SIPX_CONFDIR)!g' \
#			-e 's!@SIPX_DBDIR@!$(SIPX_DBDIR)!g' \
#			-e 's!@SIPX_LIBDIR@!$(SIPX_LIBDIR)!g' \
#			-e 's!@HTTP_ROOTDIR@!$(HTTP_ROOTDIR)!g' \
#		<$< >$@"

$(SIPX_DBDIR)/credential.xml: $(SIPX_DBDIR)/credential.xml.in
	$(SUDO) sh -c \
		"yes 1234 | update-passtokens $< $@"
