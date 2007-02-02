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
require 'utils/terminator'

# Obtains CSEs from and puts them into CSE queue
class CseReader < Dao

  def initialize(database_url, purge_age, polling_interval, log = nil)
    super(database_url, purge_age, log)
    @last_read_time = nil
    @stop = Terminator.new(polling_interval)
  end
  
  # Another way of fetching the row
  #        while row = sth.fetch_scroll(DBI::SQL_FETCH_NEXT)
  #          cse_queue << CseReader.cse_from_row(row)
  #        end
  # If stop time is nil reader will keep on reading the connection forever
  def run(cse_queue, start_time, stop_time = nil)
    connect do | dbh |
      if stop_time
        check_purge(dbh)
        read_cses(dbh, cse_queue, start_time, stop_time)
      else
        @last_read_time ||= start_time
        loop do
          check_purge(dbh)
          first = @last_read_time
          log.debug("Read CSEs from #{first}")
          read_cses(dbh, cse_queue, first, nil)
          log.debug("Going to sleep.")
          break if @stop.wait
          log.debug("Waking up.")
        end
      end        
    end
  end
  
  def stop
    @stop.stop
  end
  
  def read_cses(dbh, cse_queue, start_time, stop_time)
    sql = CseReader.select_sql(start_time, stop_time)
    params = [start_time, stop_time].find_all { | i | i }
    # FIXME: converting Timestamp to strings for now, not sure why it's not working without conversion yet
    params = params.collect{ | i | i.to_s }
    dbh.prepare(sql) do | sth |
      sth.execute(*params)
      sth.fetch do |row|
        cse = CseReader.cse_from_row(row)
        @last_read_time = cse.event_time
        cse_queue << cse
      end
    end    
  end
  
  def purge_now(dbh, start_time_cse)
    log.debug("Purging CSEs older than #{start_time_cse}")
    sql = CseReader.delete_sql(nil, start_time_cse)
    dbh.prepare(sql) do | sth |
      sth.execute(start_time_cse)
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
