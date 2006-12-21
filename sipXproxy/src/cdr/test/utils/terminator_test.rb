#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'test/unit'

$:.unshift File.join(File.dirname(__FILE__), '..', '..', 'lib')

require 'utils/terminator'

class TerminatorTest < Test::Unit::TestCase
  def test_stop
    t = Terminator.new(30)
    t.stop()
    assert(t.wait)
  end


  def test_timeout
    t = Terminator.new(0.3)
    wait_result = nil
    test_thread = Thread.new(t) do |t| 
      wait_result = t.wait()
    end
    test_thread.join()
    assert_not_nil(wait_result)
    assert(!wait_result)
  end
  
  def test_stop_while_waiting
    t = Terminator.new(0.5)
    wait_result = nil
    test_thread = Thread.new(t) do |t| 
      # wait wi
      wait_result = t.wait()
    end
    sleep(0.1)
    t.stop()
    test_thread.join()
    assert_not_nil(wait_result)
    assert(wait_result)
  end
end
