#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'dbi'

require 'call_state_event'
require 'db/dao'

# Obtains CSEs from and puts them into CSE queue
class CseReader < Dao
  
  def initialize(database_url, log = nil)
    super
  end
  
  # Another way of fetching the row
  #        while row = sth.fetch_scroll(DBI::SQL_FETCH_NEXT)
  #          cse_queue << CseReader.cse_from_row(row)
  #        end  
  def run(cse_queue, start_time, stop_time)    
    connect do | dbh |
      sql = CseReader.select_sql(start_time, stop_time)
      params = [start_time, stop_time].find_all { | i | i }
      dbh.prepare(sql) do | sth |
        sth.execute(*params)
        sth.fetch do |row|
          cse_queue << CseReader.cse_from_row(row)
        end
      end
    end
  end
  
  # purge records in CSE table
  def purge(start_time_cse)
    connect do | dbh |
      sql = CseReader.delete_sql(nil, start_time_cse)
      dbh.prepare(sql) do | sth |
        sth.execute(start_time_cse)
      end  
    end    
  end
  
  class << self
    def cse_from_row(row)
      cse = CallStateEvent.new
      CallStateEvent::FIELDS.each_with_index do
        | field, i |
        setter = (field.to_s + "=").to_sym
        cse.send( setter, row[i] ) 
      end
      return cse    
    end  
    
    def select_sql(start_time = nil, end_time = nil)
      field_names = CallStateEvent::FIELDS.collect { | f | f.to_s }
      select_str = field_names.join(', ')
      sql = "SELECT #{select_str} FROM call_state_events"
      sql = append_where_clause(sql, start_time, end_time)
      sql += " ORDER BY event_time"
    end
    
    def delete_sql(start_time, end_time)
      sql  = "DELETE FROM call_state_events"
      append_where_clause(sql, start_time, end_time)      
    end
    
    def append_where_clause(sql, start_time, end_time)
      if start_time || end_time
        sql += " WHERE"
        sql += " event_time >= ?" if start_time
        sql += " AND" if start_time && end_time
        sql += " event_time <= ?" if end_time
      end
      return sql      
    end
  end
end
