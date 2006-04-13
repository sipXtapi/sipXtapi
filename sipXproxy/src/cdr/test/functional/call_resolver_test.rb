#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

$SOURCE_DIR = File.dirname(__FILE__)    # directory in which this file is located

# system requirements
require 'parsedate'
require File.join($SOURCE_DIR, '..', 'test_helper')

# application requirements
require File.join($SOURCE_DIR, '..', '..', 'call_resolver')


# :TODO: Make it easy to run all the unit tests, possibly via Rakefile, for build loop.
class CallResolverTest < Test::Unit::TestCase
  fixtures :call_state_events, :parties, :cdrs
  
  TEST_AOR = 'aor'
  TEST_CONTACT = 'contact'
  TEST_CALL_ID = 'call ID'
  TEST_FROM_TAG = 'f'
  TEST_TO_TAG = 't'
 
  SECONDS_IN_A_DAY = 24 * 60 * 60
 
public

  def setup
    # Create the CallResolver, giving it the location of the test config file.
    @resolver = CallResolver.new(File.join($SOURCE_DIR, 'data/callresolver-config'))
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
    assert_equal(4, call_ids.length, 'Wrong number of call IDs')
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
  
  def test_start_cdr
    cdr_data = @resolver.send(
                 :start_cdr,
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
    tags = @resolver.send(:best_call_leg, events)
    to_tag = tags[1]
    assert_equal('t', to_tag, 'Wrong to_tag for best call leg')
    
    # load events for the complicated case
    call_id = 'testComplicatedSuccess'
    events = @resolver.send(:load_events, call_id)
     
    tags = @resolver.send(:best_call_leg, events)
    to_tag = tags[1]
    assert_equal('t2', to_tag, 'Wrong to_tag for best call leg')
    
    # try again, drop the final call_end event
    tags = @resolver.send(:best_call_leg, events[0..4])
    to_tag = tags[1]
    assert_equal('t1', to_tag, 'Wrong to_tag for best call leg')
    
    # try again with three events
    tags = @resolver.send(:best_call_leg, events[0..2])
    to_tag = tags[1]
    assert_equal('t0', to_tag, 'Wrong to_tag for best call leg')
    
    # try again with two events
    tags = @resolver.send(:best_call_leg, events[0..1])
    to_tag = tags[1]
    assert_equal('t0', to_tag, 'Wrong to_tag for best call leg')
    
    # try again with just the call request
    tags = @resolver.send(:best_call_leg, events[0..0])
    to_tag = tags[1]
    assert_nil(to_tag, 'Wrong to_tag for best call leg')
  end

  def test_finish_cdr
    events = load_simple_success_events
    
    # fill in cdr_data with info from the events
    to_tag = 't'
    from_tag = 'f'
    cdr_data = CdrData.new
    status = @resolver.send(:finish_cdr, cdr_data, events, from_tag, to_tag)
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
    # This set of events has call request, call setup, call failed.
    call_id = 'testFailed'
    events = @resolver.send(:load_events, call_id)
    check_failed_call(events, to_tag)
    
    # Try again without the call setup event.
    events.delete_if {|event| event.call_setup?}
    check_failed_call(events, to_tag)
  end
  
  def test_finish_cdr_callee_hangs_up
    events = load_simple_success_events_callee_hangs_up
    
    # fill in cdr_data with info from the events
    to_tag = 't'
    from_tag = 'f'
    cdr_data = CdrData.new
    status = @resolver.send(:finish_cdr, cdr_data, events, from_tag, to_tag)
    assert_equal(true, status)
    
    # define variables for cdr_data components
    cdr = cdr_data.cdr
    caller = cdr_data.caller
    callee = cdr_data.callee
    
    # Check that the CDR is filled in as expected.  It will only be partially
    # filled in because we are testing just one part of the process.
    assert_equal(to_tag, cdr.to_tag, 'Wrong to_tag')
    assert_equal(Time.parse('1990-05-17T19:41:00.000Z'), cdr.connect_time,
                            'Wrong connect_time')
    assert_equal(Time.parse('1990-05-17T19:50:00.000Z'), cdr.end_time,
                            'Wrong end_time')
    assert_equal('sip:bob@2.2.2.2', callee.contact, 'Wrong callee contact')
    assert_equal(Cdr::CALL_COMPLETED_TERM, cdr.termination, 'Wrong termination code')
    assert_nil(cdr.failure_status)
    assert_nil(cdr.failure_reason)
  end

  # Helper method for test_finish_cdr.  Check that failure info has been filled
  # in properly.
  def check_failed_call(events, to_tag)
    cdr_data = CdrData.new
    status = @resolver.send(:finish_cdr, cdr_data, events, 'f', to_tag)
    assert_equal(true, status, 'Finishing the CDR failed')
    cdr = cdr_data.cdr
    assert_equal(Cdr::CALL_FAILED_TERM, cdr.termination, 'Wrong termination code')
    assert_equal(499, cdr.failure_status)
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
  
  def test_save_cdr
    # For a clean test, make sure there is no preexisting CDR with the call
    # ID that we are using.
    Cdr.delete_all("call_id = '#{TEST_CALL_ID}'")
    
    # Create a new complete CDR.  Fill in mandatory fields so we don't get
    # database integrity exceptions on save. 
    cdr = Cdr.new(:call_id =>     TEST_CALL_ID,
                  :from_tag =>    TEST_FROM_TAG,
                  :to_tag =>      TEST_TO_TAG,
                  :caller_id =>   1,
                  :callee_id =>   2,
                  :termination => 'C')
    
    # Try to save it and confirm that it was saved.  Use a clone so we don't
    # modify the original and can reuse it.
    saved_cdr = @resolver.send(:save_cdr, cdr.clone)
    assert(saved_cdr.id, 'ID of object saved to database must be non-nil')
    
    # Try to save another clone.  We should get back the CDR that is already in
    # the database.  The clone should not be saved because we have not passed in
    # replace=true (second optional input to save_cdr) and the existing CDR is
    # complete.
    cdr2 = cdr.clone
    save_again_cdr = @resolver.send(:save_cdr, cdr2)
    assert_equal(saved_cdr.id, save_again_cdr.id)
    assert(cdr2.new_record?, 'Cdr record should not have been saved')
    
    # Pass in replace=true, now the save should happen.  Tweak cdr2 so we can
    # check that it worked.
    cdr2.termination = 'I'
    saved_cdr = @resolver.send(:save_cdr, cdr2, true)
    assert_equal(cdr2.termination, saved_cdr.termination,
                 'Cdr record was not saved even though replace=true')
    
    # Try to save another clone without setting replace=true.  Should work
    # because the saved CDR is incomplete.
    cdr3 = cdr.clone
    cdr3.termination = 'R'    # tweak the CDR so we can verify the save
    saved_cdr = @resolver.send(:save_cdr, cdr3, true)
    assert_equal(cdr3.termination, saved_cdr.termination,
      'Cdr record was not saved, should have replaced incomplete CDR')    
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
  
  def test_find_cdr
    # Find a CDR we know is in the DB
    cdr_to_find = Cdr.new(:call_id => 'call1', :from_tag => 'f1', :to_tag => 't1')
    cdr = @resolver.send(:find_cdr, cdr_to_find)
    assert(cdr, "Couldn't find CDR")
    
    # Try to find a CDR we know is not in the DB
    cdr_to_find.call_id = 'extra_bogus_call_id'
    cdr = @resolver.send(:find_cdr, cdr_to_find)
    assert_nil(cdr, "Found a CDR that shouldn't exist")
    
    # Trigger an ArgumentError
    cdr_to_find.call_id = nil
    assert_raise(ArgumentError) {cdr = @resolver.send(:find_cdr, cdr_to_find)}
  end
  
  def test_save_cdr_data
    # Try to save a CDR with a dialog ID that already exists in the DB =>
    # should just get back the existing CDR.
    cdr = Cdr.new(:call_id => 'call1', :from_tag => 'f1', :to_tag => 't1')
    caller = Party.new(:aor => 'sip:alice@example.com',
                       :contact => 'sip:alice@1.1.1.1')
    callee = Party.new(:aor => 'sip:bob@example.com',
                       :contact => 'sip:bob@2.2.2.2')
    cdr_data = CdrData.new(cdr, caller, callee)
    cdr_count_before_save = Cdr.count
    @resolver.send(:save_cdr_data, cdr_data)
    assert_equal(cdr_count_before_save, Cdr.count,
                 'The CDR count must not increase')
    
    # Save a new CDR => it should end up in the DB
    call_id2 = 'call2'
    from_tag2 = 'f2'
    to_tag2 = 't2'
    cdr = Cdr.new(:call_id => call_id2, :from_tag => from_tag2, :to_tag => to_tag2)
    cdr_data.cdr = cdr
    @resolver.send(:save_cdr_data, cdr_data)
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
    @resolver.send(:save_cdr_data, cdr_data)
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
    assert_equal(4, Cdr.count, 'Wrong number of CDRs')
  end
  
  def test_log_level_from_name
    CallResolver::LOG_LEVEL_MAP.each do |key, value|
      assert_equal(value, @resolver.send(:log_level_from_name, key))
    end
    assert_nil(@resolver.send(:log_level_from_name, 'Unknown log level name'))
  end
  
  def test_set_log_console_config
    # Pass in an empty config, should get the default value of false
    assert(!@resolver.send(:set_log_console_config, {}))
    
    # Pass in ENABLE, get true
    # Pass in DISABLE, get false
    # Comparison is case-insensitive
    assert(@resolver.send(:set_log_console_config,
      {CallResolver::LOG_CONSOLE_CONFIG => 'EnAbLe'}))
    assert(!@resolver.send(:set_log_console_config,
      {CallResolver::LOG_CONSOLE_CONFIG => 'dIsAbLe'}))
    
    # Pass in bogus value, get exception
    assert_raise(ConfigException) do
      @resolver.send(:set_log_console_config,
        {CallResolver::LOG_CONSOLE_CONFIG => 'jacket'})
    end
  end
  
  def test_set_log_dir_config
    ENV[CallResolver::SIPX_PREFIX] = nil
    
    # Pass in an empty config, should get the default log dir value
    assert_equal(CallResolver::LOG_DIR_CONFIG_DEFAULT,
                 @resolver.send(:set_log_dir_config, {}))
    
    # Set $SIPX_PREFIX and try again, this time the prefix should be added
    prefix = '/test_prefix_ignore_this_error_message'
    ENV[CallResolver::SIPX_PREFIX] = prefix
    assert_equal(File.join(prefix, CallResolver::LOG_DIR_CONFIG_DEFAULT),
                 @resolver.send(:set_log_dir_config, {}))
    
    # Configure the dir
    log_dir = 'No\phone\I\just\want\to\be\alone\today'
    assert_equal(log_dir,
                 @resolver.send(:set_log_dir_config,
                                {CallResolver::LOG_DIR_CONFIG => log_dir}))
  end
  
  def test_set_log_level_config
    # Pass in an empty config, should get the default log dir value
    assert_equal(Logger::INFO,
                 @resolver.send(:set_log_level_config, {}))
    
    # Pass in level names, get the right values
    CallResolver::LOG_LEVEL_MAP.each do |key, value|
      assert_equal(value,
                   @resolver.send(:set_log_level_config,
                                  {CallResolver::LOG_LEVEL_CONFIG => key}))
    end
    
    # Don't allow unknown log levels
    assert_raise(CallResolverException) do
      @resolver.send(:set_log_level_config,
                     {CallResolver::LOG_LEVEL_CONFIG => 'Unknown log level name'})
    end
  end

  def test_set_daily_run_config
    # Pass in an empty config, should get the default value of false
    assert(!@resolver.send(:set_daily_run_config, {}))
    
    # Pass in ENABLE, get true
    # Pass in DISABLE, get false
    # Comparison is case-insensitive
    assert(@resolver.send(:set_daily_run_config,
      {CallResolver::DAILY_RUN => 'EnAbLe'}))
    assert(!@resolver.send(:set_daily_run_config,
      {CallResolver::DAILY_RUN => 'dIsAbLe'}))
    
    # Pass in bogus value, get exception
    assert_raise(ConfigException) do
      @resolver.send(:set_daily_run_config,
        {CallResolver::DAILY_RUN => 'jacket'})
    end
  end

  def test_get_daily_start_time
    # Get today's date, cut out the date and paste our start time into
    # a time string
    today = Time.now
    todayString = today.strftime("%m/%d/%YT")
    startString = todayString + CallResolver::DAILY_RUN_TIME
    # Convert to time, start same time yesterday
    daily_start_time = Time.parse(startString)
    daily_end_time = daily_start_time
    daily_start_time -= SECONDS_IN_A_DAY   # subtract one day's worth of seconds

    # Pass in an empty config, should get the default value always since this
    # parameter is not configurable
    assert(daily_start_time == @resolver.send(:get_daily_start_time, {}))
  end

  # Test that contact params are stripped off of the contact URLs recorded in CDRs
  def test_contact_param_stripping
    start_time = Time.parse('2001-1-3T00:00:00.000Z')
    end_time = Time.parse('2001-1-3T00:00:00.000Z')
    @resolver.resolve(start_time, end_time)
    
    assert(Party.find(:first,
                      :conditions => "contact = 'sip:alice_with_params@1.1.1.1'"))
    assert(Party.find(:first,
                      :conditions => "contact = '<sip:bob_with_params@2.2.2.2>'"))
   end
  
  def test_set_purge_enable_config
    # Pass in an empty config, should get the default value of true
    assert(@resolver.send(:set_purge_enable_config, {}))
    
    # Pass in ENABLE, get true
    # Pass in DISABLE, get false
    # Comparison is case-insensitive
    assert(@resolver.send(:set_purge_enable_config,
      {CallResolver::PURGE_ENABLE => 'EnAbLe'}))
    assert(!@resolver.send(:set_purge_enable_config,
      {CallResolver::PURGE_ENABLE => 'dIsAbLe'}))
    
    # Pass in bogus value, get exception
    assert_raise(ConfigException) do
      @resolver.send(:set_purge_enable_config,
        {CallResolver::PURGE_ENABLE => 'jacket'})
    end
  end
  
  def test_get_purge_start_time
    # Get the default purge time: today's date minus the default age
    purge_start_time =
      Time.now - (SECONDS_IN_A_DAY * CallResolver::PURGE_AGE_DEFAULT.to_i)

    # Pass in an empty config, should get the default purge time, allow
    # for 1 second difference in times
    assert((@resolver.send(:get_purge_start_time, {}) - purge_start_time) < 1) 

    purgeAgeStr = '23'
    purgeAge = purgeAgeStr.to_i
    
    # Get today's date minus different age
    purge_start_time = Time.now - (SECONDS_IN_A_DAY * purgeAge)

    # Pass in a value, allow for 1 second difference in times
    assert((@resolver.send(:get_purge_start_time,
      {CallResolver::PURGE_AGE => purgeAgeStr}) - purge_start_time) < 1)
  end  
  
  #-----------------------------------------------------------------------------
  # Helper methods
  
  # load and return events for the simple case
  def load_simple_success_events
    call_id = 'testSimpleSuccess'
    @resolver.send(:load_events, call_id)
  end
 
  # load and return events for the simple case
  def load_simple_success_events_callee_hangs_up
    call_id = 'testSimpleSuccess_CalleeEnd'
    @resolver.send(:load_events, call_id)
  end  
  
end
