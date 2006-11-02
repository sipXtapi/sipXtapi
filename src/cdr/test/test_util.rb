#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

module TestUtil
  
  class DummyQueue
    attr_reader :counter, :last
    
    def initialize()
      @counter = 0
    end
    
    def enq(x)
      @last = x 
      @counter += 1
    end
  end
  
  class NullLog
    def error(*args)
    end
    
    def debug(*args)
    end
  end
  
  # creates simple mocks from klass and argument hash
  def TestUtil.create_mock(klass, args)
    o = klass.new
    
    if(args)
      args.each { |field, value| 
        setter = (field.to_s + "=").to_sym
        o.send( setter, value ) 
      }
    end    
    return o
  end
    
end
