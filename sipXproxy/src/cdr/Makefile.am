callresolver_dir=@bindir@/callresolver

# To generate a new list
#   find -type f -printf "\t%h/%f \\\\\n" \
#      egrep -v "(\.svn|Makefile.*)" \
#      sed -e 's,\.\/,    ,g' \
#       sort > Makefile.guts

callresolver_files = \
  call_resolver.rb \
  configure.rb \
  exceptions.rb \
  main.rb \
  utils.rb \
  app/controllers/application.rb \
  app/models/call_state_event.rb \
  app/models/cdr.rb \
  app/models/party.rb \
  config/boot.rb \
  config/database.yml \
  config/environment.rb \
  config/routes.rb \
  config/environments/development.rb \
  config/environments/production.rb \
  config/environments/test.rb \
  log/test.log \
  test/test_helper.rb \
  test/fixtures/call_state_events.yml \
  test/fixtures/cdrs.yml \
  test/fixtures/parties.yml \
  test/functional/call_resolver_test.rb

EXTRA_DIST = \
  $(callresolver_files)
    

install-data-hook: install-callresolver

.PHONY: install-callresolver

install-callresolver: $(callresolver_dir) $(foreach rbfile,$(callresolver_files),$(DESTDIR)$(callresolver_dir)/$(rbfile))

$(foreach rbfile,$(callresolver_files),$(DESTDIR)$(callresolver_dir)/$(rbfile)) : $(DESTDIR)$(callresolver_dir)/% : $(srcdir)/%
	$(INSTALL) -D -m 644 $< $@

$(callresolver_dir):
	$(INSTALL) -d -m755 $(DESTDIR)$(callresolver_dir)
