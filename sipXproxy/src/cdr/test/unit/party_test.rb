#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require File.dirname(__FILE__) + '/../test_helper'

class PartyTest < Test::Unit::TestCase
  fixtures :parties

  # Replace this with your real tests.
  def test_truth
    assert_kind_of Party, parties(:first)
  end
end
