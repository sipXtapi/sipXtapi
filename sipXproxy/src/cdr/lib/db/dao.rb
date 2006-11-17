#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'dbi'

module DBI
  class Timestamp
    include Comparable
    
    def <=>(other)
      result = to_time <=> other.to_time
      return result unless result == 0
      return fraction <=> other.fraction
    end
    
    def -(other)
      return to_time <=> other.to_time
    end
    
  end
end


class Dao
  attr_reader :log
  
  def initialize(database_url, log)
    @connection = database_url.to_dbi
    @username = database_url.username
    @log = log
  end
  
  def connect(&block)
    DBI.connect(@connection, @username, &block)
  end  
end