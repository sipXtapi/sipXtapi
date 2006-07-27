# Generated from activesupport-1.2.5.gem by gem2spec -*- rpm-spec -*-
%define rbname activesupport
%define version 1.2.5
%define release 1

Summary: Support and utility classes used by the Rails framework.
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
BuildRequires: ruby 
BuildRequires: ruby-gems >= 0.8.11
BuildArch: noarch
Provides: ruby(Activesupport) = %{version}

%define gemdir /usr/lib/ruby/gems/1.8
%define gembuilddir %{buildroot}%{gemdir}

%description
Utility library which carries commonly used classes and goodies from the Rails framework
%prep
%setup -T -c

%build

%install
%{__rm} -rf %{buildroot}
mkdir -p %{gembuilddir}
gem install --local --install-dir %{gembuilddir} --force --rdoc %{SOURCE0}

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root)
%{gemdir}/gems/activesupport-1.2.5/CHANGELOG
%{gemdir}/gems/activesupport-1.2.5/lib/active_support.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/binding_of_caller.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/breakpoint.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/class_attribute_accessors.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/class_inheritable_attributes.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/clean_logger.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/dependencies.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/inflections.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/inflector.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/module_attribute_accessors.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/ordered_options.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/version.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/whiny_nil.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/array.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/blank.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/cgi.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/date.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/enumerable.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/exception.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/hash.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/integer.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/kernel.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/load_error.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/numeric.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/object_and_class.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/range.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/string.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/time.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/array/conversions.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/cgi/escape_skipping_slashes.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/date/conversions.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/hash/indifferent_access.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/hash/keys.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/hash/reverse_merge.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/integer/even_odd.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/integer/inflections.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/numeric/bytes.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/numeric/time.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/range/conversions.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/string/access.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/string/conversions.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/string/inflections.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/string/starts_ends_with.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/time/calculations.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/core_ext/time/conversions.rb
%{gemdir}/gems/activesupport-1.2.5/lib/active_support/values/time_zone.rb
%doc %{gemdir}/doc/activesupport-1.2.5
%{gemdir}/cache/activesupport-1.2.5.gem
%{gemdir}/specifications/activesupport-1.2.5.gemspec

%changelog
* Sat Jan 28 2006 David Lutterkort <dlutter@redhat.com> - 1.2.5-1
- Initial build
