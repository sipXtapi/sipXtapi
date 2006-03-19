#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# system requirements
require File.dirname(__FILE__) + '/../test_helper'
require 'parsedate'

# application requirements
require '../../call_resolver'


class CallResolverTest < Test::Unit::TestCase
  fixtures :call_state_events

  def setup
    @resolver = CallResolver.new
  end

  def test_load_call_ids
    start_time = Time.parse('1990-05-17T07:30:00.000Z')
    end_time = Time.parse('1990-05-17T07:45:00.000Z')

    # Load call IDs.  Do a low level message send to bypass access control on 
    # this private method.
    call_ids = @resolver.send(:load_call_ids, start_time, end_time)
    
    # call IDs can come back in any order, so sort them to guarantee order
    call_ids.sort!
    
    # verify results
    assert_equal(2, call_ids.length, 'Wrong number of call IDs')
    assert_equal('testSimpleSuccess',                      call_ids[0], 'Wrong call ID')
    assert_equal('testSimpleSuccessBogusCallInTimeWindow', call_ids[1], 'Wrong call ID')
  end
  
  def test_load_events
    # load events
    events = @resolver.send(:load_events, 'testSimpleSuccess')
    
    # verify results
    assert_equal(3, events.length)
    events.each_index {|index| assert_equal(events[index].event_seq, index + 1)}
  end
  
end
