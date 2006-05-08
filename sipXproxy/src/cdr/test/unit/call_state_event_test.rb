#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# set up the load path
$thisdir = File.dirname(__FILE__)
$:.unshift($thisdir)
$:.unshift(File.join($thisdir, ".."))
$:.unshift(File.join($thisdir, "..", ".."))

# system requires
require File.join('test_helper')

# application requires
require 'call_state_event'


class CallStateEventTest < Test::Unit::TestCase
  fixtures :call_state_events

  def setup
    simple_success_event_names =
      [:testSimpleSuccess_1, :testSimpleSuccess_2, :testSimpleSuccess_3]
    @simple_success_events =
      simple_success_event_names.collect {|name| call_state_events(name)}
  end

  def test_load_call_state_events
    @simple_success_events.each do |event|
      assert_kind_of(CallStateEvent, event)
    end
  end

  def test_caller_aor
    @simple_success_events.each do |event|
      assert_equal('sip:alice@example.com', event.caller_aor)
    end
  end

  def test_callee_aor
    @simple_success_events.each do |event|
      assert_equal('sip:bob@example.com', event.callee_aor)
    end
  end

end
