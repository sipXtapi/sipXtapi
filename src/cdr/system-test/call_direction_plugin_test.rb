#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# system requirements
require 'parsedate'

# application requirements
$:.unshift(File.join(File.dirname(__FILE__), "..", ".."))
require 'call_resolver'
require 'call_direction_plugin'

class CallDirectionPluginTest < Test::Unit::TestCase 
public

  def setup
    # Create the CallResolver, giving it the location of the test config file.
    @resolver = CallResolver.new(File.join(File.dirname(__FILE__), 'data/callresolver-config'))
    @plugin = CallDirectionPlugin.new(@resolver)
  end

  def test_set_call_direction_config
    # Pass in an empty config, should get the default value of false
    assert(!CallDirectionPlugin.call_direction?({}))
    
    # Pass in ENABLE, get true
    # Pass in DISABLE, get false
    # Comparison is case-insensitive
    assert(CallDirectionPlugin.call_direction?(
      {CallDirectionPlugin::CALL_DIRECTION => 'EnAbLe'}))
    assert(!CallDirectionPlugin.call_direction?(
      {CallDirectionPlugin::CALL_DIRECTION => 'dIsAbLe'}))
    
    # Pass in bogus value, get exception
    assert_raise(ConfigException) do
      CallDirectionPlugin.call_direction?(
        {CallDirectionPlugin::CALL_DIRECTION => 'jacket'})
    end
  end
  
  def test_call_direction
    # Set up a single test gateway
    @plugin.send(:gateway_addresses=, ["1.1.1.1"])
    
    # Resolve calls, computing call direction via the plugin. Calls will be
    # loaded in call ID order: testAnotherSuccess1, testAnotherSuccess2, then
    # testFailed.
    Cdr.delete_all
    start_time = Time.parse('2001-1-1T00:00:00.000Z')
    end_time = Time.parse('2001-1-1T20:01:00.000Z')
    @resolver.add_observer(@plugin)
    @resolver.resolve(start_time, end_time)
    assert_equal(3, Cdr.count, 'Wrong number of CDRs')
    
    # Check that call direction came out as expected
    cdrs = Cdr.find(:all)
    check_call_direction(cdrs)
  end
  
  def test_call_direction_with_null_contact
    # Set up a single test gateway
    @plugin.send(:gateway_addresses=, ["1.1.1.1"])
    
    # Resolve the test call
    start_time = Time.parse('2001-1-2T00:00:00.000Z')
    end_time = Time.parse('2001-1-2T01:01:00.000Z')
    @resolver.add_observer(@plugin)
    @resolver.resolve(start_time, end_time)    

    # Check that call direction came out as expected
    cdr = Cdr.find(:first,
                   :conditions => ["start_time = :start_time", {:start_time => start_time}])
    assert(cdr, "Resolved the test call but didn't get a CDR")
    assert_equal(CallDirectionPlugin::INTRANETWORK, cdr.call_direction)
  end
  
  def test_weird_name_stripping
    # Set up a single test gateway
    @plugin.send(:gateway_addresses=, ["1.1.1.1"])
    
    # Resolve the test call
    start_time = Time.parse('2001-1-4T00:00:00.000Z')
    end_time = Time.parse('2001-1-4T03:01:00.000Z')
    @resolver.add_observer(@plugin)
    @resolver.resolve(start_time, end_time)    

    # Check that call direction came out as expected
    cdr = Cdr.find(:first,
                   :conditions => ["start_time = :start_time", {:start_time => start_time}])
    assert(cdr, "Resolved the test call but didn't get a CDR")
    assert_equal(CallDirectionPlugin::INCOMING, cdr.call_direction)
  end
  
  def test_domain_name_contact
    # Set up a single test gateway
    @plugin.send(:gateway_addresses=, ["1.1.1.1", "gateway.example.com"])
    
    # Resolve the test call
    start_time = Time.parse('2001-1-5T00:00:00.000Z')
    end_time = Time.parse('2001-1-5T03:01:00.000Z')
    @resolver.add_observer(@plugin)
    @resolver.resolve(start_time, end_time)    

    # Check that call direction came out as expected
    cdr = Cdr.find(:first,
                   :conditions => ["start_time = :start_time", {:start_time => start_time}])
    assert(cdr, "Resolved the test call but didn't get a CDR")
    assert_equal(CallDirectionPlugin::INCOMING, cdr.call_direction)
  end          
  
  def check_call_direction(cdrs)
    cdrs.each do |cdr|
      call_direction = cdr.call_direction
      case cdr.call_id
      when 'testAnotherSuccess1'
        assert_equal('I', call_direction)
      when 'testAnotherSuccess2'
        assert_equal('O', call_direction)
      when 'testAnotherSuccess3'
        assert_equal('A', call_direction)
      else
        assert(false, "Unexpected call ID #{cdr.call_id}")
      end
    end
  end
  
end
