%define rbname gems
%define version 0.8.11
%define release 2

%define gem_dir %(ruby -rrbconfig -e 'puts Config::CONFIG["libdir"]')/ruby/gems
%define rb_ver %(ruby -rrbconfig -e 'puts Config::CONFIG["ruby_version"]')
%define ruby_libdir %(ruby -rrbconfig -e 'puts Config::CONFIG["libdir"]')/ruby/%{rb_ver}

Summary: RubyGems is the Ruby standard for publishing and managing third party libraries.
Name: ruby-%{rbname}

Version: %{version}
Release: %{release}
Group: Development/Ruby
License: GPL
URL: http://rubyforge.org/projects/rubygems/
Source0: http://rubyforge.org/frs/download.php/1399/rubygems-%{version}.tgz
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Requires: ruby
#Requires: irb
BuildRequires: ruby
#BuildRequires: irb
BuildArch: noarch
Provides: ruby(Gems) = 0.8.11

%description
RubyGems is the Ruby standard for publishing and managing third party 
libraries.

%prep
%setup -q -n rubygems-0.8.11

%build
CFLAGS="$RPM_OPT_FLAGS" \
ruby setup.rb config --rbdir='$std-ruby' --sodir='$std-ruby/%{rb_arch}'
ruby setup.rb setup
ruby doc/makedoc.rb

%install
%{__rm} -rf %{buildroot}
GEM_HOME=$RPM_BUILD_ROOT%{_libdir}/ruby/gems/%{rb_ver}/ \
    ruby setup.rb install --prefix=$RPM_BUILD_ROOT

%clean
%{__rm} -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc README TODO ChangeLog Releases examples gemspecs
%{_bindir}/gem
%{_bindir}/gem_mirror
%{_bindir}/gem_server
%{_bindir}/gemwhich
%{_bindir}/generate_yaml_index.rb
%{_bindir}/update_rubygems

%dir %{gem_dir}
%dir %{gem_dir}/%{rb_ver}/cache
%dir %{gem_dir}/%{rb_ver}/gems
%dir %{gem_dir}/%{rb_ver}/specifications

%dir %{gem_dir}/%{rb_ver}/gems/sources-0.0.1
%dir %{gem_dir}/%{rb_ver}/gems/sources-0.0.1/lib

%{gem_dir}/%{rb_ver}/cache/sources-0.0.1.gem
%{gem_dir}/%{rb_ver}/gems/sources-0.0.1/lib/sources.rb
%{gem_dir}/%{rb_ver}/specifications/sources-0.0.1.gemspec

%dir %{ruby_libdir}/rubygems

%{ruby_libdir}/gemconfigure.rb
%{ruby_libdir}/ubygems.rb
%{ruby_libdir}/rubygems.rb
%{ruby_libdir}/rubygems/builder.rb
%{ruby_libdir}/rubygems/cmd_manager.rb
%{ruby_libdir}/rubygems/command.rb
%{ruby_libdir}/rubygems/config_file.rb
%{ruby_libdir}/rubygems/custom_require.rb
%{ruby_libdir}/rubygems/dependency_list.rb
%{ruby_libdir}/rubygems/deployment.rb
%{ruby_libdir}/rubygems/doc_manager.rb
%{ruby_libdir}/rubygems/format.rb
%{ruby_libdir}/rubygems/gem_commands.rb
%{ruby_libdir}/rubygems/gem_openssl.rb
%{ruby_libdir}/rubygems/gem_runner.rb
%{ruby_libdir}/rubygems/installer.rb
%{ruby_libdir}/rubygems/loadpath_manager.rb
%{ruby_libdir}/rubygems/old_format.rb
%{ruby_libdir}/rubygems/open-uri.rb
%{ruby_libdir}/rubygems/package.rb
%{ruby_libdir}/rubygems/remote_installer.rb
%{ruby_libdir}/rubygems/rubygems_version.rb
%{ruby_libdir}/rubygems/security.rb
%{ruby_libdir}/rubygems/source_index.rb
%{ruby_libdir}/rubygems/specification.rb
%{ruby_libdir}/rubygems/timer.rb
%{ruby_libdir}/rubygems/user_interaction.rb
%{ruby_libdir}/rubygems/validator.rb
%{ruby_libdir}/rubygems/version.rb

%changelog
* Mon Jan  9 2006 David Lutterkort <dlutter@redhat.com> - 0.8.11-1
- Updated for 0.8.11

* Sun Oct 10 2004 Omar Kilani <omar@tinysofa.org> 0.8.1-1ts
- First version of the package 
