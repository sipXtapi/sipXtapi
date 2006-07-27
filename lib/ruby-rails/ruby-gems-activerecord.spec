# Generated from activerecord-1.13.2.gem by gem2spec -*- rpm-spec -*-
%define rbname activerecord
%define version 1.13.2
%define release 1

Summary: Implements the ActiveRecord pattern for ORM.
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
Requires: ruby-gems-activesupport = 1.2.5
BuildRequires: ruby 
BuildRequires: ruby-gems >= 0.8.11
BuildArch: noarch
Provides: ruby(Activerecord) = %{version}

%define gemdir /usr/lib/ruby/gems/1.8
%define gembuilddir %{buildroot}%{gemdir}

%description
Implements the ActiveRecord pattern (Fowler, PoEAA) for ORM. It ties database tables and classes together for business objects, like Customer or Subscription, that can find, save, and destroy themselves without resorting to manual SQL.
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
%{gemdir}/gems/activerecord-1.13.2/rakefile
%{gemdir}/gems/activerecord-1.13.2/install.rb
%doc %{gemdir}/gems/activerecord-1.13.2/README
%{gemdir}/gems/activerecord-1.13.2/RUNNING_UNIT_TESTS
%{gemdir}/gems/activerecord-1.13.2/CHANGELOG
%{gemdir}/gems/activerecord-1.13.2/lib/active_record.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/aggregations.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/associations.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/base.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/callbacks.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/deprecated_associations.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/deprecated_finders.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/fixtures.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/locking.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/migration.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/observer.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/query_cache.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/reflection.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/schema.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/schema_dumper.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/timestamp.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/transactions.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/validations.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/version.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/wrappings.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/acts/list.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/acts/nested_set.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/acts/tree.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/associations/association_collection.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/associations/association_proxy.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/associations/belongs_to_association.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/associations/has_and_belongs_to_many_association.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/associations/has_many_association.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/associations/has_one_association.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/abstract_adapter.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/db2_adapter.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/firebird_adapter.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/mysql_adapter.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/oci_adapter.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/postgresql_adapter.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/sqlite_adapter.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/sqlserver_adapter.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/abstract/connection_specification.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/abstract/database_statements.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/abstract/quoting.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/abstract/schema_definitions.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/connection_adapters/abstract/schema_statements.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/vendor/db2.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/vendor/mysql.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/vendor/simple.rb
%{gemdir}/gems/activerecord-1.13.2/lib/active_record/wrappers/yaml_wrapper.rb
%{gemdir}/gems/activerecord-1.13.2/test/aaa_create_tables_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/abstract_unit.rb
%{gemdir}/gems/activerecord-1.13.2/test/active_schema_mysql.rb
%{gemdir}/gems/activerecord-1.13.2/test/adapter_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/aggregations_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/all.sh
%{gemdir}/gems/activerecord-1.13.2/test/ar_schema_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/association_callbacks_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/association_inheritance_reload.rb
%{gemdir}/gems/activerecord-1.13.2/test/associations_extensions_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/associations_go_eager_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/associations_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/base_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/binary_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/callbacks_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/class_inheritable_attributes_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/column_alias_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/copy_table_sqlite.rb
%{gemdir}/gems/activerecord-1.13.2/test/default_test_firebird.rb
%{gemdir}/gems/activerecord-1.13.2/test/deprecated_associations_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/deprecated_finder_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/finder_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/inheritance_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/lifecycle_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/locking_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/method_scoping_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/migration_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/mixin_nested_set_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/mixin_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/modules_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/multiple_db_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/pk_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/readonly_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/reflection_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/schema_dumper_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/schema_test_postgresql.rb
%{gemdir}/gems/activerecord-1.13.2/test/synonym_test_oci.rb
%{gemdir}/gems/activerecord-1.13.2/test/threaded_connections_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/transactions_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/unconnected_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/validations_test.rb
%{gemdir}/gems/activerecord-1.13.2/test/connections/native_db2/connection.rb
%{gemdir}/gems/activerecord-1.13.2/test/connections/native_firebird/connection.rb
%{gemdir}/gems/activerecord-1.13.2/test/connections/native_mysql/connection.rb
%{gemdir}/gems/activerecord-1.13.2/test/connections/native_oci/connection.rb
%{gemdir}/gems/activerecord-1.13.2/test/connections/native_postgresql/connection.rb
%{gemdir}/gems/activerecord-1.13.2/test/connections/native_sqlite/connection.rb
%{gemdir}/gems/activerecord-1.13.2/test/connections/native_sqlite3/connection.rb
%{gemdir}/gems/activerecord-1.13.2/test/connections/native_sqlite3/in_memory_connection.rb
%{gemdir}/gems/activerecord-1.13.2/test/connections/native_sqlserver/connection.rb
%{gemdir}/gems/activerecord-1.13.2/test/connections/native_sqlserver_odbc/connection.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/accounts.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/author.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/authors.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/auto_id.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/binary.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/categories.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/categories_ordered.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/categories_posts.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/category.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/column_name.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/comment.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/comments.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/companies.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/company.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/company_in_module.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/computer.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/computers.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/course.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/courses.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/customer.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/customers.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/default.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/developer.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/developers.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/developers_projects.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/entrant.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/entrants.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/fixture_database.sqlite
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/fixture_database_2.sqlite
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/fk_test_has_fk.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/fk_test_has_pk.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/flowers.jpg
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/keyboard.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/mixin.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/mixins.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/movie.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/movies.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/order.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/people.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/person.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/post.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/posts.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/project.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/projects.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/reply.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/subject.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/subscriber.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/task.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/tasks.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/topic.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/topics.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/bad_fixtures/attr_with_numeric_first_char
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/bad_fixtures/attr_with_spaces
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/bad_fixtures/blank_line
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/bad_fixtures/duplicate_attributes
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/bad_fixtures/missing_value
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/create_oracle_db.bat
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/create_oracle_db.sh
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/db2.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/db2.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/db22.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/db22.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/firebird.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/firebird.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/firebird2.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/firebird2.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/mysql.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/mysql.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/mysql2.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/mysql2.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/oci.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/oci.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/oci2.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/oci2.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/postgresql.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/postgresql.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/postgresql2.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/postgresql2.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/sqlite.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/sqlite.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/sqlite2.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/sqlite2.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/sqlserver.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/sqlserver.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/sqlserver2.drop.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/db_definitions/sqlserver2.sql
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/developers_projects/david_action_controller
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/developers_projects/david_active_record
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/developers_projects/jamis_active_record
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/migrations/1_people_have_last_names.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/migrations/2_we_need_reminders.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/migrations/3_innocent_jointable.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/migrations_with_duplicate/1_people_have_last_names.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/migrations_with_duplicate/2_we_need_reminders.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/migrations_with_duplicate/3_foo.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/migrations_with_duplicate/3_innocent_jointable.rb
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/naked/csv/accounts.csv
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/naked/yml/accounts.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/naked/yml/companies.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/naked/yml/courses.yml
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/subscribers/first
%{gemdir}/gems/activerecord-1.13.2/test/fixtures/subscribers/second
%{gemdir}/gems/activerecord-1.13.2/examples/associations.png
%{gemdir}/gems/activerecord-1.13.2/examples/associations.rb
%{gemdir}/gems/activerecord-1.13.2/examples/shared_setup.rb
%{gemdir}/gems/activerecord-1.13.2/examples/validation.rb
%doc %{gemdir}/doc/activerecord-1.13.2
%{gemdir}/cache/activerecord-1.13.2.gem
%{gemdir}/specifications/activerecord-1.13.2.gemspec

%changelog
* Sat Jan 28 2006 David Lutterkort <dlutter@redhat.com> - 1.13.2-1
- Initial build
