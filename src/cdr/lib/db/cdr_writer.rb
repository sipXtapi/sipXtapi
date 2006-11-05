#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'cdr'

# Writes CDRs to the database
class CdrWriter
  
  def initialize(queue, database_url)
    @queue = queue
    @connection = database_url.to_dbi
    @username = database_url.username    
  end
  
  def connect(&block)
    DBI.connect(@connection, @username, &block)
  end  
  
  def run
    connect do | dbh |
      sql = CdrWriter.insert_sql
      dbh.prepare(sql) do | sth |
        while cdr = @queue.shift
          row = CdrWriter.row_from_cdr(cdr)
          sth.execute(*row)
        end
      end
    end  
  end
  
  def purge(start_time_cdr)
    connect do | dbn |
      sql = CdrWriter.delete_sql
      dbh.prepare(sql) do | sth |
        sth.execute(start_time_cdr)
      end  
    end    
  end
  
  class << self
    def row_from_cdr(cdr)
      Cdr::FIELDS.collect { | f | cdr.send(f) }
    end
    
    def insert_sql()
      field_names = Cdr::FIELDS.collect { | f | f.to_s }
      field_str = field_names.join(', ')
      value_str = (['?'] * field_names.size).join(', ')
      sql = "INSERT INTO cdrs ( #{field_str} ) VALUES ( #{value_str} )"
    end        
    
    def delete_sql
      sql  = "DELETE FROM cdrs WHERE ? < start_time"
    end
  end
  
end
