# Generated from rake-0.7.0.gem by gem2spec -*- rpm-spec -*-
%define rbname rake
%define version 0.7.0
%define release 1

Summary: Ruby based make-like utility.
Name: ruby-gems-%{rbname}

Version: %{version}
Release: %{release}
Group: Development/Ruby
License: MIT
URL: http://rake.rubyforge.org
Source0: %{rbname}-%{version}.gem
# Make sure the spec template is included in the SRPM
Source1: ruby-gems-%{rbname}.spec.in
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Requires: ruby 
Requires: ruby-gems >= 0.8.11
BuildRequires: ruby 
BuildRequires: ruby-gems >= 0.8.11
BuildArch: noarch
Provides: ruby(Rake) = %{version}

%define gemdir /usr/lib/ruby/gems/1.8
%define gembuilddir %{buildroot}%{gemdir}

%description
Rake is a Make-like program implemented in Ruby. Tasks and dependencies are specified in standard Ruby syntax.
%prep
%setup -T -c

%build

%install
%{__rm} -rf %{buildroot}
mkdir -p %{gembuilddir}
gem install --local --install-dir %{gembuilddir} --force --rdoc %{SOURCE0}
mkdir -p %{buildroot}/%{_bindir}
mv %{gembuilddir}/bin/* %{buildroot}/%{_bindir}
rmdir %{gembuilddir}/bin

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root)
%{_bindir}/rake
%{gemdir}/gems/rake-0.7.0/install.rb
%doc %{gemdir}/gems/rake-0.7.0/CHANGES
%{gemdir}/gems/rake-0.7.0/Rakefile
%doc %{gemdir}/gems/rake-0.7.0/README
%doc %{gemdir}/gems/rake-0.7.0/TODO
%doc %{gemdir}/gems/rake-0.7.0/MIT-LICENSE
%{gemdir}/gems/rake-0.7.0/bin/rake
%{gemdir}/gems/rake-0.7.0/lib/rake.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/clean.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/tasklib.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/packagetask.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/rdoctask.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/runtest.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/testtask.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/gempackagetask.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/ruby182_test_unit_fix.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/rake_test_loader.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/classic_namespace.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/contrib/ftptools.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/contrib/sys.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/contrib/compositepublisher.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/contrib/publisher.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/contrib/rubyforgepublisher.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/contrib/sshpublisher.rb
%{gemdir}/gems/rake-0.7.0/lib/rake/loaders/makefile.rb
%{gemdir}/gems/rake-0.7.0/test/filecreation.rb
%{gemdir}/gems/rake-0.7.0/test/test_filelist.rb
%{gemdir}/gems/rake-0.7.0/test/test_fileutils.rb
%{gemdir}/gems/rake-0.7.0/test/test_package_task.rb
%{gemdir}/gems/rake-0.7.0/test/test_ftp.rb
%{gemdir}/gems/rake-0.7.0/test/test_clean.rb
%{gemdir}/gems/rake-0.7.0/test/shellcommand.rb
%{gemdir}/gems/rake-0.7.0/test/test_tasks.rb
%{gemdir}/gems/rake-0.7.0/test/functional.rb
%{gemdir}/gems/rake-0.7.0/test/test_rake.rb
%{gemdir}/gems/rake-0.7.0/test/test_makefile_loader.rb
%{gemdir}/gems/rake-0.7.0/test/test_test_task.rb
%{gemdir}/gems/rake-0.7.0/test/session_functional.rb
%{gemdir}/gems/rake-0.7.0/test/test_rules.rb
%{gemdir}/gems/rake-0.7.0/test/test_file_creation_task.rb
%{gemdir}/gems/rake-0.7.0/test/test_file_task.rb
%{gemdir}/gems/rake-0.7.0/test/test_earlytime.rb
%{gemdir}/gems/rake-0.7.0/test/test_multitask.rb
%{gemdir}/gems/rake-0.7.0/test/test_definitions.rb
%{gemdir}/gems/rake-0.7.0/test/test_task_manager.rb
%{gemdir}/gems/rake-0.7.0/test/test_namespace.rb
%{gemdir}/gems/rake-0.7.0/test/contrib/testsys.rb
%{gemdir}/gems/rake-0.7.0/test/data/rbext/rakefile.rb
%{gemdir}/gems/rake-0.7.0/test/data/sample.mf
%{gemdir}/gems/rake-0.7.0/test/data/imports/deps.mf
%{gemdir}/gems/rake-0.7.0/test/data/default/Rakefile
%{gemdir}/gems/rake-0.7.0/test/data/multidesc/Rakefile
%{gemdir}/gems/rake-0.7.0/test/data/chains/Rakefile
%{gemdir}/gems/rake-0.7.0/test/data/dryrun/Rakefile
%{gemdir}/gems/rake-0.7.0/test/data/imports/Rakefile
%{gemdir}/gems/rake-0.7.0/test/data/namespace/Rakefile
%{gemdir}/gems/rake-0.7.0/test/data/file_creation_task/Rakefile
%doc %{gemdir}/gems/rake-0.7.0/doc/glossary.rdoc
%doc %{gemdir}/gems/rake-0.7.0/doc/proto_rake.rdoc
%{gemdir}/gems/rake-0.7.0/doc/jamis.rb
%doc %{gemdir}/gems/rake-0.7.0/doc/rational.rdoc
%doc %{gemdir}/gems/rake-0.7.0/doc/rakefile.rdoc
%{gemdir}/gems/rake-0.7.0/doc/rake.1.gz
%{gemdir}/gems/rake-0.7.0/doc/example/Rakefile1
%{gemdir}/gems/rake-0.7.0/doc/example/Rakefile2
%{gemdir}/gems/rake-0.7.0/doc/example/a.c
%{gemdir}/gems/rake-0.7.0/doc/example/b.c
%{gemdir}/gems/rake-0.7.0/doc/example/main.c
%doc %{gemdir}/gems/rake-0.7.0/doc/release_notes/rake-0.4.14.rdoc
%doc %{gemdir}/gems/rake-0.7.0/doc/release_notes/rake-0.5.3.rdoc
%doc %{gemdir}/gems/rake-0.7.0/doc/release_notes/rake-0.4.15.rdoc
%doc %{gemdir}/gems/rake-0.7.0/doc/release_notes/rake-0.6.0.rdoc
%doc %{gemdir}/gems/rake-0.7.0/doc/release_notes/rake-0.5.0.rdoc
%doc %{gemdir}/gems/rake-0.7.0/doc/release_notes/rake-0.5.4.rdoc
%doc %{gemdir}/gems/rake-0.7.0/doc/release_notes/rake-0.7.0.rdoc
%doc %{gemdir}/doc/rake-0.7.0
%{gemdir}/cache/rake-0.7.0.gem
%{gemdir}/specifications/rake-0.7.0.gemspec

%changelog
* Sat Jan 28 2006 David Lutterkort <dlutter@redhat.com> - 0.7.0-1
- Initial build

