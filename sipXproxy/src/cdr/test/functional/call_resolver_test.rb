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


class PartyTest < Test::Unit::TestCase
  fixtures :call_state_events

  def test_load_call_state_events
    resolver = CallResolver.new
    start_time = DateTime.parse('1990-05-17T07:30:00.000Z')
    end_time = DateTime.parse('1990-05-17T07:45:00.000Z')

    # Load events.  Do a low level message send to bypass access control on 
    # this private method.
    events = resolver.send(:load_call_state_events, start_time, end_time)

    assert_equal(3, events.length, "Loaded the wrong number of events")
  end

end
