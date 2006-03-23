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
  fixtures :call_state_events, :parties, :cdrs
  
  TEST_AOR = 'aor'
  TEST_CONTACT = 'contact'
 
public

  def setup
    @resolver = CallResolver.new
  end

  def test_load_call_ids
    start_time = Time.parse('1990-05-17T19:30:00.000Z')
    end_time = Time.parse('1990-05-17T19:45:00.000Z')

    # Load call IDs.  Do a low level message send to bypass access control on 
    # this private method.
    call_ids = @resolver.send(:load_call_ids, start_time, end_time)
    
    # call IDs can come back in any order, so sort them to guarantee order
    call_ids.sort!
    
    # verify results
    assert_equal(2, call_ids.length, 'Wrong number of call IDs')
    assert_equal('testSimpleSuccess',                      call_ids[0], 'Wrong call ID')
    assert_equal('testSimpleSuccessBogusCallInTimeWindow', call_ids[1], 'Wrong call ID')
  end
  
  def test_load_events
    events = load_simple_success_events
    assert_equal(3, events.length)
    events.each_index do |index|
      assert_equal(events[index].event_seq, index + 1, 'Events are not in the right order')
      assert_equal('testSimpleSuccess', events[index].call_id)
    end
  end
  
  def test_find_first_call_request
    events = load_simple_success_events
     
    # find the first call_request event
    call_req = @resolver.send(:find_first_call_request, events)
    assert_not_nil(call_req, 'No call request event found')
    assert_equal(CallStateEvent::CALL_REQUEST_TYPE, call_req.event_type,
                 'Wrong event type, not a call request: ' + call_req.event_type)
    assert_equal(1, call_req.event_seq,
                 "Wrong call request sequence number #{call_req.event_seq}")
  end
  
  def test_read_call_request
    cdr_data = @resolver.send(
                 :read_call_request,
                 call_state_events('testSimpleSuccess_1'))
    cdr = cdr_data.cdr
    caller = cdr_data.caller
    callee = cdr_data.callee
    
    # verify the caller
    assert_equal('sip:alice@example.com', caller.aor)
    assert_equal('sip:alice@1.1.1.1', caller.contact)
    
    # verify the callee: we have the AOR but not the contact
    assert_equal('sip:bob@example.com', callee.aor)
    assert_nil(callee.contact)

    # verify the CDR
    assert_equal('testSimpleSuccess', cdr.call_id)
    assert_equal('f', cdr.from_tag)
    assert_nil(cdr.to_tag)    # don't have the to tag yet
    assert_equal(Time.parse('1990-05-17T19:30:00.000Z'), cdr.start_time)
    assert_equal(Cdr::CALL_REQUESTED_TERM, cdr.termination)
  end

  def test_best_call_leg
    events = load_simple_success_events
     
    # Pick the call leg with the best outcome and longest duration to be the
    # basis for the CDR.
    to_tag = @resolver.send(:best_call_leg, events)
    assert_equal('t', to_tag, 'Wrong to_tag for best call leg')
    
    # load events for the complicated case
    call_id = 'testComplicatedSuccess'
    events = @resolver.send(:load_events, call_id)
     
    to_tag = @resolver.send(:best_call_leg, events)
    assert_equal('t2', to_tag, 'Wrong to_tag for best call leg')
    
    # try again, drop the final call_end event
    to_tag = @resolver.send(:best_call_leg, events[0..4])
    assert_equal('t1', to_tag, 'Wrong to_tag for best call leg')
    
    # try again with three events
    to_tag = @resolver.send(:best_call_leg, events[0..2])
    assert_equal('t0', to_tag, 'Wrong to_tag for best call leg')
    
    # try again with two events
    to_tag = @resolver.send(:best_call_leg, events[0..1])
    assert_equal('t0', to_tag, 'Wrong to_tag for best call leg')
    
    # try again with just the call request
    to_tag = @resolver.send(:best_call_leg, events[0..0])
    assert_nil(to_tag, 'Wrong to_tag for best call leg')
  end

  def test_create_cdr
    events = load_simple_success_events
    
    # fill in cdr_data with info from the events
    to_tag = 't'
    cdr_data = CdrData.new
    status = @resolver.send(:create_cdr, cdr_data, events, to_tag)
    assert_equal(true, status)
    
    # define variables for cdr_data components
    cdr = cdr_data.cdr
    caller = cdr_data.caller
    callee = cdr_data.callee
    
    # Check that the CDR is filled in as expected.  It will only be partially
    # filled in because we are testing just one part of the process.
    assert_equal(to_tag, cdr.to_tag, 'Wrong to_tag')
    assert_equal(Time.parse('1990-05-17T19:31:00.000Z'), cdr.connect_time,
                            'Wrong connect_time')
    assert_equal(Time.parse('1990-05-17T19:40:00.000Z'), cdr.end_time,
                            'Wrong end_time')
    assert_equal('sip:bob@2.2.2.2', callee.contact, 'Wrong callee contact')
    assert_equal(Cdr::CALL_COMPLETED_TERM, cdr.termination, 'Wrong termination code')
    assert_nil(cdr.failure_status)
    assert_nil(cdr.failure_reason)
    
    # Test a failed call.  Check only that the failure info has been filled in
    # properly.  We've checked other info in the case above.
    call_id = 'testFailed'
    events = @resolver.send(:load_events, call_id)
    cdr_data = CdrData.new
    status = @resolver.send(:create_cdr, cdr_data, events, to_tag)
    assert_equal(true, status)
    cdr = cdr_data.cdr
    assert_equal(Cdr::CALL_FAILED_TERM, cdr.termination, 'Wrong termination code')
    assert_equal(486, cdr.failure_status)
    assert_equal("You Can't Always Get What You Want", cdr.failure_reason) 
  end
 
  def test_find_party
    # For a clean test, make sure there is no preexisting Party with the aor
    # and contact that we are using.
    Party.delete_all("aor = '#{TEST_AOR}' and contact = '#{TEST_CONTACT}'")
    
    # Create a Party
    party = Party.new(:aor => 'aor', :contact => 'contact')
    
    # The Party shouldn't be in the DB
    assert_nil(@resolver.send(:find_party, party))
               
    # Save the Party, now it should be in the DB
    party.save
    assert_equal(party, @resolver.send(:find_party, party),
                 'Could not find the saved Party in the database')
  end
  
  def test_save_party_if_new
    # For a clean test, make sure there is no preexisting Party with the aor
    # and contact that we are using.
    Party.delete_all("aor = '#{TEST_AOR}' and contact = '#{TEST_CONTACT}'")
    
    # Create a Party
    party = Party.new(:aor => 'aor', :contact => 'contact')
    
    # Save it
    saved_party = @resolver.send(:save_party_if_new, party)
    assert_equal(saved_party, party)
    
    # Make a new Party just like the last one.  When we try to save it, we
    # should get back the one that is already in the DB.
    party2 = Party.new(:aor => 'aor', :contact => 'contact')
    save_again_party = @resolver.send(:save_party_if_new, party2)
    assert_equal(saved_party.id, save_again_party.id)
    assert(party2.new_record?, 'Party record should not have been saved')
  end
  
  def test_find_cdr_by_dialog
    cdr = @resolver.send(:find_cdr_by_dialog, 'call1', 'f1', 't1')
    assert(cdr, "Couldn't find CDR")
    cdr = @resolver.send(:find_cdr_by_dialog, 'extra_bogus_call_id', 'f1', 't1')
    assert_nil(cdr, "Found a CDR that shouldn't exist")
  end
  
  def test_save_cdr
    # Try to save a CDR with a dialog ID that already exists in the DB =>
    # should just get back the existing CDR.
    cdr = Cdr.new(:call_id => 'call1', :from_tag => 'f1', :to_tag => 't1')
    caller = Party.new(:aor => 'sip:alice@example.com',
                       :contact => 'sip:alice@1.1.1.1')
    callee = Party.new(:aor => 'sip:bob@example.com',
                       :contact => 'sip:bob@2.2.2.2')
    cdr_data = CdrData.new(cdr, caller, callee)
    cdr_count_before_save = Cdr.count
    @resolver.send(:save_cdr, cdr_data)
    assert_equal(cdr_count_before_save, Cdr.count,
                 'The CDR count must not increase')
    
    # Save a new CDR => it should end up in the DB
    call_id2 = 'call2'
    from_tag2 = 'f2'
    to_tag2 = 't2'
    cdr = Cdr.new(:call_id => call_id2, :from_tag => from_tag2, :to_tag => to_tag2)
    cdr_data.cdr = cdr
    @resolver.send(:save_cdr, cdr_data)
    assert_equal(cdr_count_before_save + 1, Cdr.count,
                 'The CDR was not saved to the database')
    cdr = @resolver.send(:find_cdr_by_dialog, call_id2, from_tag2, to_tag2)
    assert_not_nil(cdr, "Didn't find saved CDR")
    
    # Save a new CDR with a new caller and callee => everything gets persisted
    call_id3 = 'call3'
    from_tag3 = 'f3'
    to_tag3 = 't3'
    cdr = Cdr.new(:call_id => call_id3, :from_tag => from_tag3, :to_tag => to_tag3)
    caller = Party.new(:aor => 'sip:varda@example.com',
                       :contact => 'sip:varda@3.3.3.3')
    callee = Party.new(:aor => 'sip:thor@example.com',
                       :contact => 'sip:thor@4.4.4.4')
    cdr_data = CdrData.new(cdr, caller, callee)
    cdr_count_before_save = Cdr.count
    party_count_before_save = Party.count
    @resolver.send(:save_cdr, cdr_data)
    assert_equal(cdr_count_before_save + 1, Cdr.count,
                 'The CDR was not saved to the database')
    assert_equal(party_count_before_save + 2, Party.count,
                 'The caller and/or callee was not saved to the database')
    cdr = @resolver.send(:find_cdr_by_dialog, call_id3, from_tag3, to_tag3)
    assert_not_nil(cdr, "Didn't find saved CDR")     
  end
  
  def test_resolve_call
    ['testSimpleSuccess', 'testComplicatedSuccess', 'testFailed'].each do |call_id|
      @resolver.send(:resolve_call, call_id)
      cdr = Cdr.find_by_call_id(call_id)
      assert_not_nil(cdr, 'CDR was not created')
    end
  end
    
  def test_resolve
    Cdr.delete_all
    start_time = Time.parse('1990-01-1T000:00:00.000Z')
    end_time = Time.parse('2000-12-31T00:00.000Z')
    @resolver.resolve(start_time, end_time)
    assert_equal(3, Cdr.count, 'Wrong number of CDRs')
  end
  
  #-----------------------------------------------------------------------------
  # Helper methods
  
  # load and return events for the simple case
  def load_simple_success_events
    call_id = 'testSimpleSuccess'
    @resolver.send(:load_events, call_id)
  end
  
end
