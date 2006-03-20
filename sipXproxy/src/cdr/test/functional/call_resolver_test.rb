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

public

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
    call_id = 'testSimpleSuccess'
    events = @resolver.send(:load_events, call_id)
    
    # verify results
    assert_equal(3, events.length)
    events.each_index do |index|
      assert_equal(events[index].event_seq, index + 1, 'Events are not in the right order')
      assert_equal(call_id, events[index].call_id)
    end
  end
  
  def test_find_first_call_request
    # load events
    call_id = 'testSimpleSuccess'
    events = @resolver.send(:load_events, call_id)
    
    # find the first call_request event
    call_req = @resolver.send(:find_first_call_request, events)
    assert_not_nil(call_req, 'No call request event found')
    assert_equal(CallStateEvent::CALL_REQUEST_TYPE, call_req.event_type,
                 'Wrong event type, not a call request: ' + call_req.event_type)
    assert_equal(1, call_req.event_seq,
                 "Wrong call request sequence number #{call_req.event_seq}")
  end
  
  def test_read_call_request(call_req)
    partial_cdr = @resolver.send(
                    :read_call_request,
                    @resolver.send(:find_first_call_request, events))
    cdr = partial_cdr.cdr
    caller = partial_cdr.caller
    callee = partial_cdr.callee
    
    # verify the caller
    assert_equal('sip:alice@example.com', caller.aor)
    assert_equal('sip:alice@1.1.1.1', caller.contact)
    
    # verify the callee: we have the AOR but not the contact
    assert_equal('sip:bob@example.com', callee.aor)
    assert_nil(callee.contact)

    # verify the CDR
    assert_equal('testSimpleSuccess', cdr.caller_id)
    assert_equal('f', cdr.from_tag)
    assert_nil(cdr.to_tag)    # don't have the to tag yet
    assert_equal(Time.parse('1990-05-17T07:30:00.000Z'), cdr.start_time)
    assert_equal(Cdr.CALL_REQUESTED_TERM, cdr.termination)
  end

end
