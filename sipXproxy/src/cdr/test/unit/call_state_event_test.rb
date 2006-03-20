#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require File.dirname(__FILE__) + '/../test_helper'
require '../../utils'

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
