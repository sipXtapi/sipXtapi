#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# set up the load path
$thisdir = File.dirname(__FILE__)
$:.unshift($thisdir)
$:.unshift(File.join($thisdir, ".."))
$:.unshift(File.join($thisdir, "..", ".."))
TEST_CONFIG_FILE = File.join($thisdir, "..", 'functional', 'data', 'callresolver-config')

# system requires
require 'test_helper'

# application requires
require 'call_resolver_configure'
require 'exceptions'
require 'utils'


class CallResolverConfigureTest < Test::Unit::TestCase

  LOCALHOST = 'localhost'

  # How many seconds are there in a day
  SECONDS_IN_A_DAY = 86400
  
  def setup
    @config = CallResolverConfigure.new(CallResolverConfigure::DEFAULT_CONFIG)
  end
  
  def test_log_level_from_name
    CallResolverConfigure::LOG_LEVEL_MAP.each do |key, value|
      assert_equal(value, @config.send(:log_level_from_name, key))
    end
    assert_nil(@config.send(:log_level_from_name, 'Unknown log level name'))
  end
  
  def test_set_log_console_config
    # Pass in an empty config, should get the default value of false
    assert(!@config.send(:set_log_console_config, {}))
    
    # Pass in ENABLE, get true
    # Pass in DISABLE, get false
    # Comparison is case-insensitive
    assert(@config.send(:set_log_console_config,
      {CallResolverConfigure::LOG_CONSOLE_CONFIG => 'EnAbLe'}))
    assert(!@config.send(:set_log_console_config,
      {CallResolverConfigure::LOG_CONSOLE_CONFIG => 'dIsAbLe'}))
    
    # Pass in bogus value, get exception
    assert_raise(ConfigException) do
      @config.send(:set_log_console_config,
        {CallResolverConfigure::LOG_CONSOLE_CONFIG => 'jacket'})
    end
  end
  
  def test_set_log_dir_config
    ENV[CallResolverConfigure::SIPX_PREFIX] = nil
    
    # Pass in an empty config, should get the default log dir value
    assert_equal(CallResolverConfigure::LOG_DIR_CONFIG_DEFAULT,
                 @config.send(:set_log_dir_config, {}))
    
    # Set $SIPX_PREFIX and try again, this time the prefix should be added
    prefix = '/test_prefix_ignore_this_error_message'
    ENV[CallResolverConfigure::SIPX_PREFIX] = prefix
    assert_equal(File.join(prefix, CallResolverConfigure::LOG_DIR_CONFIG_DEFAULT),
                 @config.send(:set_log_dir_config, {}))
    
    # Configure the dir
    log_dir = 'No\phone\I\just\want\to\be\alone\today'
    assert_equal(log_dir,
                 @config.send(:set_log_dir_config,
                                {CallResolverConfigure::LOG_DIR_CONFIG => log_dir}))
  end
  
  def test_set_log_level_config
    # Pass in an empty config, should get the default log dir value
    assert_equal(Logger::INFO,
                 @config.send(:set_log_level_config, {}))
    
    # Pass in level names, get the right values
    CallResolverConfigure::LOG_LEVEL_MAP.each do |key, value|
      assert_equal(value,
                   @config.send(:set_log_level_config,
                                  {CallResolverConfigure::LOG_LEVEL_CONFIG => key}))
    end
    
    # Don't allow unknown log levels
    assert_raise(CallResolverException) do
      @config.send(:set_log_level_config,
                     {CallResolverConfigure::LOG_LEVEL_CONFIG => 'Unknown log level name'})
    end
  end

  def test_set_daily_run_config
    # Pass in an empty config, should get the default value of false
    assert(!@config.send(:set_daily_run_config, {}))
    
    # Pass in ENABLE, get true
    # Pass in DISABLE, get false
    # Comparison is case-insensitive
    assert(@config.send(:set_daily_run_config,
      {CallResolverConfigure::DAILY_RUN => 'EnAbLe'}))
    assert(!@config.send(:set_daily_run_config,
      {CallResolverConfigure::DAILY_RUN => 'dIsAbLe'}))
    
    # Pass in bogus value, get exception
    assert_raise(ConfigException) do
      @config.send(:set_daily_run_config,
        {CallResolverConfigure::DAILY_RUN => 'jacket'})
    end
  end

  def test_set_daily_start_time_config
    # Get today's date, cut out the date and paste our start time into
    # a time string
    today = Time.now
    todayString = today.strftime("%m/%d/%YT")
    startString = todayString + CallResolverConfigure::DAILY_RUN_TIME
    # Convert to time, start same time yesterday
    daily_start_time = Time.parse(startString)
    daily_end_time = daily_start_time
    daily_start_time -= SECONDS_IN_A_DAY   # subtract one day's worth of seconds

    # Pass in an empty config, should get the default value always since this
    # parameter is not configurable
    assert(daily_start_time == @config.send(:set_daily_start_time_config, {}))
  end

  def test_set_purge_config
    # Pass in an empty config, should get the default value of true
    assert(@config.send(:set_purge_config, {}))
    
    # Pass in ENABLE, get true
    # Pass in DISABLE, get false
    # Comparison is case-insensitive
    assert(@config.send(:set_purge_config,
      {CallResolverConfigure::PURGE => 'EnAbLe'}))
    assert(!@config.send(:set_purge_config,
      {CallResolverConfigure::PURGE => 'dIsAbLe'}))
    
    # Pass in bogus value, get exception
    assert_raise(ConfigException) do
      @config.send(:set_purge_config,
        {CallResolverConfigure::PURGE => 'jacket'})
    end
  end
  
  def test_set_purge_start_time_cdr_config
    # Get the default purge time: today's date minus the default age
    purge_start_time_cdr =
      Time.now - (SECONDS_IN_A_DAY * CallResolverConfigure::PURGE_AGE_CDR_DEFAULT)

    # Pass in an empty config, should get the default purge time, allow
    # for 1 second difference in times
    assert((@config.send(:set_purge_start_time_cdr_config, {}) - purge_start_time_cdr) < 1) 

    purgeAgeStr = '23'
    purgeAge = purgeAgeStr.to_i
    
    # Get today's date minus different age
    purge_start_time_cdr = Time.now - (SECONDS_IN_A_DAY * purgeAge)

    # Pass in a value, allow for 1 second difference in times
    assert((@config.send(:set_purge_start_time_cdr_config,
      {CallResolverConfigure::PURGE_AGE_CDR => purgeAgeStr}) - purge_start_time_cdr) < 1)
  end  
  
  def test_set_purge_start_time_cse_config
    # Get the default purge time: today's date minus the default age
    purge_start_time_cse =
      Time.now - (SECONDS_IN_A_DAY * CallResolverConfigure::PURGE_AGE_CSE_DEFAULT)

    # Pass in an empty config, should get the default purge time, allow
    # for 1 second difference in times
    assert((@config.send(:set_purge_start_time_cse_config, {}) - purge_start_time_cse) < 1) 

    purgeAgeStr = '23'
    purgeAge = purgeAgeStr.to_i
    
    # Get today's date minus different age
    purge_start_time_cse = Time.now - (SECONDS_IN_A_DAY * purgeAge)

    # Pass in a value, allow for 1 second difference in times
    assert((@config.send(:set_purge_start_time_cse_config,
      {CallResolverConfigure::PURGE_AGE_CSE => purgeAgeStr}) - purge_start_time_cse) < 1)
  end    
  
  def test_cse_database_urls
    # Clear the CSE DB URLs to ensure that they will get recreated
    @config.send(:cse_database_urls=, nil)
    
    # Create URLs given the host port list
    host_port_list = [1, 2, 3]
    @config.send(:host_port_list=, host_port_list)
    urls = @config.send(:cse_database_urls)
    assert_equal(3, urls.size)
    urls.each_with_index do |url, i|
      assert_equal(DatabaseUrl::DATABASE_DEFAULT, url.database)
      assert_equal(host_port_list[i], url.port)
      assert_equal(LOCALHOST, url.host)
    end

    # If the host port list is empty or nil, then we should get the default URL
    [nil, []].each do |host_port_list|
      @config.send(:cse_database_urls=, nil)
      @config.send(:host_port_list=, host_port_list)
      urls = @config.send(:cse_database_urls)
      assert_equal(1, urls.size)
      url = urls[0]
      assert_equal(DatabaseUrl::DATABASE_DEFAULT, url.database)
      assert_equal(DatabaseUrl::DATABASE_PORT_DEFAULT, url.port)
      assert_equal(LOCALHOST, url.host)
    end
  end
  
  def test_set_cse_hosts_config
    # Get the default entry - array with length 1 and default port
    port_array = @config.send(:set_cse_hosts_config, {})
    
    assert(port_array.length == 1, 'Wrong number of entries in array')
    assert(port_array[0] == DatabaseUrl::DATABASE_PORT_DEFAULT , 'Wrong port number')
    assert(!@config.send(:ha?))
    
    # Pass in other list, no localhost
    hostString = 'test.example.com:5433, test2.example.com:5434'
    port_array = @config.send(:set_cse_hosts_config, 
      {CallResolverConfigure::CSE_HOSTS => hostString})    
      
    assert(port_array.length == 2, 'Wrong number of entries in array')
    assert(@config.send(:ha?))
    assert(port_array[0] == 5433, 'Wrong port number in first entry')
    assert(port_array[1] == 5434, 'Wrong port number in second entry')      

    # Pass in other list, localhost, no port
    hostString = 'test.example.com:5433,localhost'
    port_array = @config.send(:set_cse_hosts_config, 
      {CallResolverConfigure::CSE_HOSTS => hostString})    
      
    assert(port_array.length == 2, 'Wrong number of entries in array')
    assert(@config.send(:ha?))
    assert(port_array[0] == 5433, 'Wrong port number in first entry')
    assert(port_array[1] == DatabaseUrl::DATABASE_PORT_DEFAULT, 'Wrong port number in second entry') 
    
    # Pass in other list, localhost, no port
    hostString = 'test.example.com:5433,localhost:6666'
    port_array = @config.send(:set_cse_hosts_config, 
      {CallResolverConfigure::CSE_HOSTS => hostString})    
      
    assert(port_array.length == 2, 'Wrong number of entries in array')
    assert(@config.send(:ha?))
    assert(port_array[0] == 5433, 'Wrong port number in first entry')
    assert(port_array[1] == 6666, 'Wrong port number in second entry')               
  end

  def test_parse_int_param
    # Check that we get the default value for an undefined param
    default = 666
    assert_equal(default, @config.send(:parse_int_param, {}, 'UNDEFINED_PARAM', default))
    
    # Check min and max constraints    
    config_param = 'PARAM'
    config = {config_param => '10'}
    # Param exceeds the max value
    assert_raise(ConfigException) do
      @config.send(:parse_int_param, config, 'PARAM', default, 0, 5)
    end
    # Param is below the min value
    assert_raise(ConfigException) do
      @config.send(:parse_int_param, config, 'PARAM', default, 15, 20)
    end
    # Param is OK
    assert_nothing_thrown do
      @config.send(:parse_int_param, config, 'PARAM', default, 0, 20)
    end

    # Param is not an integer, should blow up
    config = {config_param => 'zax'}
    assert_raise(ConfigException) do
      @config.send(:parse_int_param, config, 'PARAM')
    end
  end

  def test_raise_config_exception
    assert_raise(ConfigException) do
      @config.send(:raise_config_exception, 'config exception')
    end    
  end
  
end
