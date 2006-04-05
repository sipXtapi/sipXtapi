#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require File.join(File.dirname(__FILE__), '..', 'test_helper')


class CdrTest < Test::Unit::TestCase
  # We must include call_directions here because of the foreign key from
  # call_directions to cdrs.  If we don't clear call_directions then we won't
  # be able to clear cdrs.
  fixtures :parties, :cdrs, :call_directions

  def test_load_cdrs
    assert_kind_of(Cdr, cdrs(:first))
  end
  
  # Test that the belongs_to declarations of foreign key relationships work.
  def test_belongs_to
    cdr = cdrs(:first)
    assert_equal('sip:alice@example.com', cdr.caller.aor)
    assert_equal('sip:bob@example.com', cdr.callee.aor)
  end
  
  def test_complete?
    cdr = Cdr.new
    
    # No termination code => CDR is not complete
    assert(!cdr.complete?)
    
    # Termination code of completion or failure implies complete CDR
    cdr.termination = Cdr::CALL_COMPLETED_TERM
    assert(cdr.complete?)
    cdr.termination = Cdr::CALL_FAILED_TERM
    assert(cdr.complete?)
    
    # Termination code of request or in progress implies incomplete CDR
    cdr.termination = Cdr::CALL_REQUESTED_TERM
    assert(!cdr.complete?)
    cdr.termination = Cdr::CALL_IN_PROGRESS_TERM
    assert(!cdr.complete?)
  end
  
end
