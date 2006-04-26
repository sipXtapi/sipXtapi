#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'ipaddr'

require File.join(File.dirname(__FILE__), 'database_url')
require File.join(File.dirname(__FILE__), 'exceptions')


# DatabaseUtils contains utility methods that are primarily for test support.
class DatabaseUtils
  # Utils has only class methods, so don't allow instantiation.
  private_class_method :new

  # Constants
  
  # Name of the PostgreSQL command-line interface executable.
  POSTGRESQL_CLI = 'psql'

  # This is the first DB created by PostgreSQL.  Use it to do meta-queries like
  # which DBs exist.
  POSTGRESQL_INITIAL_DATABASE = 'template1'
  
  POSTGRESQL_SCHEMA_FILE = File.join(File.dirname(__FILE__), '..', '..', 'etc', 'database', 'schema.sql')
    
  # Query to list PostgreSQL DBs.
  # See http://www.postgresql.org/docs/8.0/static/managing-databases.html#MANAGE-AG-OVERVIEW .
  QUERY_LIST_DATABASES = 'SELECT datname FROM pg_database;'
  
public

  # Execute a SQL command, accessing the database
  def DatabaseUtils.exec_sql(sql, db_name = POSTGRESQL_INITIAL_DATABASE)
    `#{POSTGRESQL_CLI} -U #{DatabaseUrl::USERNAME_DEFAULT} -d #{db_name} -c "#{sql}"`
  end

  # Execute SQL commands on the database, loading commands from a file
  def DatabaseUtils.exec_sql_file(file, db_name = POSTGRESQL_INITIAL_DATABASE)
    `#{POSTGRESQL_CLI} -U #{DatabaseUrl::USERNAME_DEFAULT} -d #{db_name} -f "#{file}"`
  end

  # Return a boolean indicating whether the named DB exists.
  # Comparison of db_name is case-sensitive.
  def DatabaseUtils.database_exists?(db_name)
    # Returns a list of DB names, each of which is followed by \n
    str = exec_sql(QUERY_LIST_DATABASES)
    
    # Find the DB name.  Including the \n terminator rules out substring matches.
    str.index(db_name + "\n") != nil
  end
  
  # Create a database instance with the specified name if one does not already
  # exist, and load the schema.
  def DatabaseUtils.create_database(db_name)
    if !database_exists?(db_name)
      `createdb -U #{DatabaseUrl::USERNAME_DEFAULT} #{db_name}`
      exec_sql_file(POSTGRESQL_SCHEMA_FILE, db_name)
    end
  end

end
