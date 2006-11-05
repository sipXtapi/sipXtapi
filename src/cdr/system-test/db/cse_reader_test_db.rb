#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'test/unit'
require 'rubygems'

$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

require 'db/database_url'
require 'db/cse_reader'

class CseReaderTest < Test::Unit::TestCase
  
  class MockQueue
    attr_reader :i
    
    def initialize
      @i = 0
    end
    
    def enq(cse)    
      if @i % 500 == 0
        p cse.event_time
      end
      @i += 1
    end
  end
  
  # this one connects to real database for now
  def test_DB
    url = DatabaseUrl.new()
    p url
    q = MockQueue.new
    reader = CseReader.new(q, url)
    reader.run(nil, nil)
    p q.i    
  end
end
