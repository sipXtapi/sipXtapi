#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require File.dirname(__FILE__) + '/../test_helper'

class CallStateEventTest < Test::Unit::TestCase
  fixtures :call_state_events

  def test_load_call_state_events
    event_names = [:testSimpleSuccess_1, :testSimpleSuccess_2, :testSimpleSuccess_3]
    event_names.each do |name|
      assert_kind_of CallStateEvent, call_state_events(name)
    end
  end

end
