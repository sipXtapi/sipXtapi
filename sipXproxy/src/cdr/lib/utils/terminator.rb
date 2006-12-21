#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'monitor'


class Terminator < Monitor
  
  def initialize(timeout)
    super()
    @timeout = timeout
    @cond = new_cond()
    @stop = false    
  end
  
  # returns true if stopped was called, returns false it there was a timeout
  def wait
    synchronize do      
      return true if @stop
      return @cond.wait(@timeout)
    end    
  end
  
  def stop
    synchronize do
      @stop = true
      @cond.signal()
    end
  end
end
