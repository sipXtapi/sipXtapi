#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'dbi'

class Dao
  def Dao.establish_connection(url)
    @@dbh = DBI.connect(url, "postgres")        
  end
  
  def Dao.find(where = nil, order = nil)
    result = []
    query = select_query(where, order)
    sth = @@dbh.execute(query)
    sth.each do |row|
      result << row_to_object(row)
    end
    sth.finish
    return result
  end
  
  def Dao.find_first(where = nil, order = nil)
    query = select_query(where, order)
    sth = @@dbh.execute(query)
    result = row_to_object(sth)
    sth.finish
    return result
  end
  
  def Dao.save(object)
    
    
  end
  
  def Dao.update(object)
    
  end
  
  def Dao.close_connection()
    @@dbh.close()    
  end
  
  def select_query(where, order)
    sql = "SELECT * FROM #{table()}" 
    sql += "WHERE #where" if where
    sql += "ORDER BY #order" if order    
    return sql  
  end
  
  def row_to_object() pass end
  def object_to_row() pass end
  def table() pass end  
end