#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# Holds the params needed to connect to a database
class DatabaseUrl < Struct.new(:database, :port, :host, :adapter, :username)
  
  DATABASE_DEFAULT = 'SIPXCDR'
  DATABASE_PORT_DEFAULT = 5432                # Default port used by PostgreSQL
  HOST_DEFAULT     = 'localhost'  
  ADAPTER_DEFAULT  = 'postgresql'
  USERNAME_DEFAULT = 'postgres'
  
  def initialize(args = nil)
    super()
    args.each do |field, value| 
        self[field] = value
    end if args
    self[:database] ||= DATABASE_DEFAULT
    self[:port] ||= DATABASE_PORT_DEFAULT
    self[:host] ||= HOST_DEFAULT
    self[:adapter] ||= ADAPTER_DEFAULT
    self[:username] ||= USERNAME_DEFAULT
  end
  
  def to_dbi
    "dbi:Pg:#{database}:#{host}"
  end
end
