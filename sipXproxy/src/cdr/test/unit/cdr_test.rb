#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require File.dirname(__FILE__) + '/../test_helper'

class CdrTest < Test::Unit::TestCase
  fixtures :cdrs

  # Replace this with your real tests.
  def test_truth
    assert_kind_of Cdr, cdrs(:first)
  end
end
