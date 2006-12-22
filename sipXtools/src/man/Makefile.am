EXTRA_DIST = \
    $(MAN_FILES) \
	$(man_MANS)

man_MANS = \
	sipx-stats.1 \
	expire-spread.1 \
	monitor-spread.1 \
	sipxcar.1 \
	sipxproc.1 \
	polycom-config.1 \
	sipdb-dump.1

MAN_FILES = $(subst .1,.txt2man,$(man_MANS))

# explicit list otherwise 
#	$(subst .txt2man,.1,$(MAN_FILES))

#
# man files are not generated everytime because txt2man
# is not likely to be installed. For those editing man
# pages should update install it and run this command 
#
#   cd src/man; make generate-man-files
#
srcdir_man_MANS = $(foreach F,$(man_MANS),$(srcdir)/$F)
generate-man-files: force-man-file-generation $(srcdir_man_MANS)

force-man-file-generation :
	cd $(srcdir); touch $(MAN_FILES)

#$(srcdir)/%.1 : $(srcdir)/%.txt2man
#	txt2man $< > $@
