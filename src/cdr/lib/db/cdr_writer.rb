#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################


# Writes CDRs to the database
class CdrWriter
  
  # cdr_queue - queue from which we will get CDRs to be written to the database
  def initialize(queue)
    @queue = queue
  end
  
  
  def run
    while cdr = @queue.deq
      p cdr
    end    
  end
  
  def purge(start_time_cdr)
    # FIXME: implement purging
  end  
end
