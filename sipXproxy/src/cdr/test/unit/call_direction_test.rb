#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# set up the load path
thisdir = File.dirname(__FILE__)
$:.unshift(thisdir)
$:.unshift(File.join(thisdir, ".."))
$:.unshift(File.join(thisdir, "..", ".."))

require 'test_helper'
require 'call_direction'


class CallDirectionTest < Test::Unit::TestCase
  fixtures :cdrs, :call_directions
  
  def test_load_call_directions
    call_dir = call_directions(:first)
    assert_kind_of(CallDirection, call_dir)
    assert_equal(1, call_dir.id)
    assert_equal('I', call_dir.direction)
  end
  
  # Test that the foreign key relationships work.
  def test_foreign_keys
    call_dir = call_directions(:first)
    cdr = call_dir.cdr
    assert_equal('call1', cdr.call_id)
    assert_equal('I', cdr.call_direction.direction)
  end
  
end
