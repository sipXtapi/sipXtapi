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

  def test_load_call_state_events
    start_time = Time.parse('1990-05-17T07:30:00.000Z')
    end_time = Time.parse('1990-05-17T07:45:00.000Z')

    # Load events.  Do a low level message send to bypass access control on 
    # this private method.
    events = @resolver.send(:load_call_state_events, start_time, end_time)

    # Verify that the events got loaded and they are in the right order
    assert_equal(4, events.length, 'Loaded the wrong number of events')
    assert_equal('aaaCallId', events[0].call_id, 'Wrong call ID')
    events[1..3].each do |event|
      assert_equal('testSimpleSuccess', event.call_id, 'Wrong call ID')
    end
    expected_times = ['1990-05-17T07:33:00.000Z', '1990-05-17T07:30:00.000Z',
                      '1990-05-17T07:31:00.000Z', '1990-05-17T07:40:00.000Z']
    events.each_index do |index|
      assert_equal(Time.parse(expected_times[index]), events[index].event_time,
                   'Wrong event time')
    end
  end
  
end
