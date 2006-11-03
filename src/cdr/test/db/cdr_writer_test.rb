#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'test/unit'
require 'thread'

$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

require 'db/cdr_writer'

class CdrWriterTest < Test::Unit::TestCase
  
  def test_empty
    q = Queue.new
    t = Thread.new(CdrWriter.new(q)) { | w | w.run }    
    q.enq(nil)
    t.join
  end
  
end
