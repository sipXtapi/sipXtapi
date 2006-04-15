#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require File.join(File.dirname(__FILE__), '..', 'test_helper')


class PartyTest < Test::Unit::TestCase
  # We must include cdrs here because of the foreign keys from cdrs to parties.
  # If we don't clear cdrs then we won't be able to clear parties.
  fixtures :parties, :cdrs

  def setup
    @party_new =
      Party.new(:aor => 'sip:alice@example.com',
                :contact => 'sip:alice@1.1.1.1')    
  end

  def test_equal
    party_db = parties('first')
    assert(@party_new == party_db,
      'Overridden "==" method must return true for parties with same aor and contact')
    assert(@party_new.eql?(party_db),
      'Overridden "eql?" method must return true for parties with same aor and contact')
  end
  
  def test_hash
    assert_equal(@party_new.hash, parties('first').hash,
      'Overridden "hash" method must return same value for parties with same' +
      'aor and contact')
    assert_not_equal(parties('second').hash, parties('first').hash,
      'Overridden "hash" method must return different value for parties with' +
      'different aor and contact')
  end
  
end
