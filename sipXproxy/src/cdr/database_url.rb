#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# DatabaseUrl holds the params needed to connect to a database, the same params
# needed for ActiveRecord::Base.establish_connection.
class DatabaseUrl
  
  DATABASE_DEFAULT = 'SIPXCDR'
  DEFAULT_DATABASE_PORT = 5432
  ADAPTER_DEFAULT  = 'postgresql'
  HOST_DEFAULT     = 'localhost'  
  USERNAME_DEFAULT = 'postgres'
  
public

  attr_accessor :adapter, :host, :username, :database, :port
  
  def initialize(database = DATABASE_DEFAULT,
                 port     = DEFAULT_DATABASE_PORT,
                 adapter  = ADAPTER_DEFAULT,
                 host     = HOST_DEFAULT,
                 username = USERNAME_DEFAULT)
    @database = database
    @port = port if port
    @adapter = adapter
    @host = host
    @username = username
  end
  
  # Return the params in a hash.  Intended for use with
  # ActiveRecord::Base.establish_connection, which takes a hash as input.
  def to_hash
    h = {:database => database,
         :port => port,
         :adapter => adapter,
         :host => host,
         :username => username}
    h 
  end
  
  def ==(url)
    (database ? (url.database and database == url.database) : !url.database) and
    (port ? (url.port and port == url.port) : !url.port) and
    (adapter ? (url.adapter and adapter == url.adapter) : !url.adapter) and
    (host ? (url.host and host == url.host) : !url.host) and
    (username ? (url.username and username == url.username) : !url.username)  
  end
  
  def eql?(url)
    self == url
  end
  
  def hash
    haash = 0
    haash += database.hash if database
    haash += port.hash if port
    haash += adapter.hash if adapter
    haash += host.hash if host
    haash += username.hash if username
    haash
  end
  
  # Print a descriptive string for the database.  Since the adapter and username
  # are currently always the same, don't print them.
  def to_s
    _database = database or '<default database>'
    _host = host or '<default host>'
    str = "#{_database} at #{_host}"
    str += ":#{port}" if port
    str
  end
  
end
