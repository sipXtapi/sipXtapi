# Generated from rails-1.0.0.gem by gem2spec -*- rpm-spec -*-
%define rbname rails
%define version 1.0.0
%define release 1

Summary: Web-application framework with template engine, control-flow layer, and ORM.
Name: ruby-gems-%{rbname}

Version: %{version}
Release: %{release}
Group: Development/Ruby
License: MIT
URL: http://www.rubyonrails.org
Source0: %{rbname}-%{version}.gem
# Make sure the spec template is included in the SRPM
Source1: ruby-gems-%{rbname}.spec.in
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Requires: ruby 
Requires: ruby-gems >= 0.8.11
Requires: ruby-gems-rake >= 0.6.2
Requires: ruby-gems-activesupport = 1.2.5
Requires: ruby-gems-activerecord = 1.13.2
Requires: ruby-gems-actionpack = 1.11.2
Requires: ruby-gems-actionmailer = 1.1.5
Requires: ruby-gems-actionwebservice = 1.0.0
BuildRequires: ruby 
BuildRequires: ruby-gems >= 0.8.11
BuildArch: noarch
Provides: ruby(Rails) = %{version}

%define gemdir /usr/lib/ruby/gems/1.8
%define gembuilddir %{buildroot}%{gemdir}

%description
Rails is a framework for building web-application using CGI, FCGI, mod_ruby, or WEBrick on top of either MySQL, PostgreSQL, SQLite, DB2, SQL Server, or Oracle with eRuby- or Builder-based templates.
%prep
%setup -T -c

%build

%install
%{__rm} -rf %{buildroot}
mkdir -p %{gembuilddir}
gem install --local --install-dir %{gembuilddir} --force %{SOURCE0}
mkdir -p %{buildroot}/%{_bindir}
mv %{gembuilddir}/bin/* %{buildroot}/%{_bindir}
rmdir %{gembuilddir}/bin

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root)
%{_bindir}/rails
%{gemdir}/gems/rails-1.0.0/CHANGELOG
%{gemdir}/gems/rails-1.0.0/fresh_rakefile
%{gemdir}/gems/rails-1.0.0/MIT-LICENSE
%{gemdir}/gems/rails-1.0.0/Rakefile
%{gemdir}/gems/rails-1.0.0/README
%{gemdir}/gems/rails-1.0.0/bin/about
%{gemdir}/gems/rails-1.0.0/bin/breakpointer
%{gemdir}/gems/rails-1.0.0/bin/console
%{gemdir}/gems/rails-1.0.0/bin/destroy
%{gemdir}/gems/rails-1.0.0/bin/generate
%{gemdir}/gems/rails-1.0.0/bin/plugin
%{gemdir}/gems/rails-1.0.0/bin/rails
%{gemdir}/gems/rails-1.0.0/bin/runner
%{gemdir}/gems/rails-1.0.0/bin/server
%{gemdir}/gems/rails-1.0.0/bin/performance/benchmarker
%{gemdir}/gems/rails-1.0.0/bin/performance/profiler
%{gemdir}/gems/rails-1.0.0/bin/process/reaper
%{gemdir}/gems/rails-1.0.0/bin/process/spawner
%{gemdir}/gems/rails-1.0.0/bin/process/spinner
%{gemdir}/gems/rails-1.0.0/builtin/controllers/rails_info_controller.rb
%{gemdir}/gems/rails-1.0.0/configs/apache.conf
%{gemdir}/gems/rails-1.0.0/configs/database.yml
%{gemdir}/gems/rails-1.0.0/configs/empty.log
%{gemdir}/gems/rails-1.0.0/configs/lighttpd.conf
%{gemdir}/gems/rails-1.0.0/configs/routes.rb
%{gemdir}/gems/rails-1.0.0/doc/README_FOR_APP
%{gemdir}/gems/rails-1.0.0/dispatches/dispatch.fcgi
%{gemdir}/gems/rails-1.0.0/dispatches/dispatch.rb
%{gemdir}/gems/rails-1.0.0/dispatches/gateway.cgi
%{gemdir}/gems/rails-1.0.0/environments/boot.rb
%{gemdir}/gems/rails-1.0.0/environments/development.rb
%{gemdir}/gems/rails-1.0.0/environments/environment.rb
%{gemdir}/gems/rails-1.0.0/environments/production.rb
%{gemdir}/gems/rails-1.0.0/environments/test.rb
%{gemdir}/gems/rails-1.0.0/helpers/application.rb
%{gemdir}/gems/rails-1.0.0/helpers/application_helper.rb
%{gemdir}/gems/rails-1.0.0/helpers/test_helper.rb
%{gemdir}/gems/rails-1.0.0/html/404.html
%{gemdir}/gems/rails-1.0.0/html/500.html
%{gemdir}/gems/rails-1.0.0/html/favicon.ico
%{gemdir}/gems/rails-1.0.0/html/index.html
%{gemdir}/gems/rails-1.0.0/html/robots.txt
%{gemdir}/gems/rails-1.0.0/html/images/rails.png
%{gemdir}/gems/rails-1.0.0/html/javascripts/controls.js
%{gemdir}/gems/rails-1.0.0/html/javascripts/dragdrop.js
%{gemdir}/gems/rails-1.0.0/html/javascripts/effects.js
%{gemdir}/gems/rails-1.0.0/html/javascripts/prototype.js
%{gemdir}/gems/rails-1.0.0/lib/binding_of_caller.rb
%{gemdir}/gems/rails-1.0.0/lib/breakpoint.rb
%{gemdir}/gems/rails-1.0.0/lib/breakpoint_client.rb
%{gemdir}/gems/rails-1.0.0/lib/code_statistics.rb
%{gemdir}/gems/rails-1.0.0/lib/commands.rb
%{gemdir}/gems/rails-1.0.0/lib/console_sandbox.rb
%{gemdir}/gems/rails-1.0.0/lib/dispatcher.rb
%{gemdir}/gems/rails-1.0.0/lib/fcgi_handler.rb
%{gemdir}/gems/rails-1.0.0/lib/initializer.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_info.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_version.rb
%{gemdir}/gems/rails-1.0.0/lib/railties_path.rb
%{gemdir}/gems/rails-1.0.0/lib/rubyprof_ext.rb
%{gemdir}/gems/rails-1.0.0/lib/test_help.rb
%{gemdir}/gems/rails-1.0.0/lib/webrick_server.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/about.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/breakpointer.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/console.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/destroy.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/generate.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/plugin.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/runner.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/server.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/update.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/ncgi/listener
%{gemdir}/gems/rails-1.0.0/lib/commands/ncgi/tracker
%{gemdir}/gems/rails-1.0.0/lib/commands/performance/benchmarker.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/performance/profiler.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/process/reaper.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/process/spawner.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/process/spinner.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/servers/lighttpd.rb
%{gemdir}/gems/rails-1.0.0/lib/commands/servers/webrick.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/base.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/commands.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/lookup.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/manifest.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/options.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/scripts.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/simple_logger.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/spec.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/applications/app/app_generator.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/applications/app/USAGE
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/controller/controller_generator.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/controller/USAGE
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/controller/templates/controller.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/controller/templates/functional_test.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/controller/templates/helper.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/controller/templates/view.rhtml
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/mailer/mailer_generator.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/mailer/USAGE
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/mailer/templates/fixture.rhtml
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/mailer/templates/mailer.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/mailer/templates/unit_test.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/mailer/templates/view.rhtml
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/migration/migration_generator.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/migration/USAGE
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/migration/templates/migration.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/model/model_generator.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/model/USAGE
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/model/templates/fixtures.yml
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/model/templates/model.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/model/templates/unit_test.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/plugin/plugin_generator.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/plugin/USAGE
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/plugin/templates/generator.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/plugin/templates/init.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/plugin/templates/plugin.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/plugin/templates/Rakefile
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/plugin/templates/README
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/plugin/templates/tasks.rake
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/plugin/templates/unit_test.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/plugin/templates/USAGE
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/scaffold_generator.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/USAGE
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/templates/controller.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/templates/form.rhtml
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/templates/form_scaffolding.rhtml
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/templates/functional_test.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/templates/helper.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/templates/layout.rhtml
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/templates/style.css
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/templates/view_edit.rhtml
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/templates/view_list.rhtml
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/templates/view_new.rhtml
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/scaffold/templates/view_show.rhtml
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/session_migration/session_migration_generator.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/session_migration/USAGE
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/session_migration/templates/migration.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/web_service/USAGE
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/web_service/web_service_generator.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/web_service/templates/api_definition.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/web_service/templates/controller.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/generators/components/web_service/templates/functional_test.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/scripts/destroy.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/scripts/generate.rb
%{gemdir}/gems/rails-1.0.0/lib/rails_generator/scripts/update.rb
%{gemdir}/gems/rails-1.0.0/lib/tasks/databases.rake
%{gemdir}/gems/rails-1.0.0/lib/tasks/documentation.rake
%{gemdir}/gems/rails-1.0.0/lib/tasks/framework.rake
%{gemdir}/gems/rails-1.0.0/lib/tasks/javascripts.rake
%{gemdir}/gems/rails-1.0.0/lib/tasks/misc.rake
%{gemdir}/gems/rails-1.0.0/lib/tasks/rails.rb
%{gemdir}/gems/rails-1.0.0/lib/tasks/statistics.rake
%{gemdir}/gems/rails-1.0.0/lib/tasks/testing.rake
%{gemdir}/cache/rails-1.0.0.gem
%{gemdir}/specifications/rails-1.0.0.gemspec

%changelog
* Sat Jan 28 2006 David Lutterkort <dlutter@redhat.com> - 1.0.0-1
- Initial build

