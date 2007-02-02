#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'test/unit'

# set up the load path
$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

require 'utils/call_resolver_configure'
require 'utils/configure'
require 'utils/exceptions'
require 'utils/sipx_logger'
require 'utils/utils'

class CallResolverConfigureTest < Test::Unit::TestCase
  
  def setup
    @config = CallResolverConfigure.from_file(CallResolverConfigure::DEFAULT_CONFIG)
  end
  
  def test_from_file
    assert_not_nil(CallResolverConfigure.from_file(nil))
  end  
  
  def test_get_log_dir_config
    ENV[CallResolverConfigure::SIPX_PREFIX] = nil
    
    # Pass in an empty config, should get the default log dir value
    assert_not_nil(@config.log)
    assert_equal(CallResolverConfigure::LOG_DIR_CONFIG_DEFAULT, @config.get_log_dir_config)
    
    # Set $SIPX_PREFIX and try again, this time the prefix should be added
    prefix = '/test_prefix_ignore_this_error_message'
    ENV[CallResolverConfigure::SIPX_PREFIX] = prefix
    assert_equal(File.join(prefix, CallResolverConfigure::LOG_DIR_CONFIG_DEFAULT),    
    @config.get_log_dir_config)
    
    
    log_dir = 'No\phone\I\just\want\to\be\alone\today'
    c = Configure.new(CallResolverConfigure::LOG_DIR_CONFIG => log_dir)
    
    # Configure the dir
    assert_equal(log_dir, CallResolverConfigure.new(c).get_log_dir_config)
  end
  
  def test_set_log_level_config
    
    # Pass in an empty config, should get the default log dir value
    assert_equal(Logger::INFO, @config.log.level)
    
    # Pass in level names, get the right values
    SipxLogger::LOG_LEVEL_SIPX_TO_LOGGER.each do |key, value|
      c = Configure.new(CallResolverConfigure::LOG_LEVEL_CONFIG => key)
      
      assert_equal(value, CallResolverConfigure.new(c).log.level)
    end
    
    # Don't allow unknown log levels
    assert_raise(CallResolverException) do
      c = Configure.new(CallResolverConfigure::LOG_LEVEL_CONFIG => 'Unknown log level name')
      CallResolverConfigure.new(c)   
    end
  end
  
  def test_set_purge_config
    # Pass in an empty config, should get the default value of true
    assert(@config.purge?)
    
    # Pass in ENABLE, get true
    # Pass in DISABLE, get false
    # Comparison is case-insensitive
    c = Configure.new({CallResolverConfigure::PURGE => 'EnAbLe'})
    assert(CallResolverConfigure.new(c).purge?)
    
    c = Configure.new({CallResolverConfigure::PURGE => 'dIsAbLe'})
    assert(!CallResolverConfigure.new(c).purge?)
    
    # Pass in bogus value, get exception
    c = Configure.new({CallResolverConfigure::PURGE => 'jacket'})
    assert_raise(ConfigException) do
      assert(CallResolverConfigure.new(c).purge?)
    end
  end
  
  def test_get_purge_start_time_cdr_config
    assert_equal(@config.purge_age_cdr, CallResolverConfigure::PURGE_AGE_CDR_DEFAULT) 
    
    c = Configure.new(CallResolverConfigure::PURGE_AGE_CDR => '23')
    config = CallResolverConfigure.new(c)
    assert(config.purge_age_cdr, 23)

    c = Configure.new(CallResolverConfigure::PURGE_AGE_CDR => '23', CallResolverConfigure::PURGE => 'DISABLE')
    config = CallResolverConfigure.new(c)
    assert_nil(config.purge_age_cdr)
  end  
  
  def test_get_purge_start_time_cse_config
    assert(@config.purge_age_cse,  CallResolverConfigure::PURGE_AGE_CDR_DEFAULT) 
    
    c = Configure.new(CallResolverConfigure::PURGE_AGE_CSE => '23')
    config = CallResolverConfigure.new(c)    
    assert(config.purge_age_cse, 23)
    
    c = Configure.new(CallResolverConfigure::PURGE_AGE_CSE => '23', CallResolverConfigure::PURGE => 'DISABLE')
    config = CallResolverConfigure.new(c)    
    assert_nil(config.purge_age_cse)    
  end    
  
  def test_cse_database_urls
    # Create URLs given the host port list
    host_port_list = [1, 2, 3]
    urls = @config.get_cse_database_urls_config(host_port_list)
    assert_equal(3, urls.size)
    urls.each_with_index do |url, i|
      assert_equal(DatabaseUrl::DATABASE_DEFAULT, url.database)
      assert_equal(host_port_list[i], url.port)
      assert_equal('localhost', url.host)
    end
    
    # If the host port list is empty, then we should get the default URL
    urls = @config.get_cse_database_urls_config([])
    assert_equal(1, urls.size)
    url = urls[0]
    assert_equal(DatabaseUrl::DATABASE_DEFAULT, url.database)
    assert_equal(DatabaseUrl::DATABASE_PORT_DEFAULT, url.port)
    assert_equal('localhost', url.host)
  end
  
  def test_get_cse_hosts_config
    # Get the default entry - array with length 1 and default port
    host_array, port_array, ha = @config.get_cse_hosts_config
    
    assert_equal(1, port_array.length)
    assert_equal(1, host_array.length)
    assert(port_array[0] == DatabaseUrl::DATABASE_PORT_DEFAULT)
    assert(!ha)
    
    # Pass in other list, no localhost
    hostString = 'test.example.com:5433, test2.example.com:5434'
    c = Configure.new(CallResolverConfigure::CSE_HOSTS => hostString)
    config = CallResolverConfigure.new(c)
    
    host_array, port_array, ha = config.get_cse_hosts_config
    
    assert_equal(2, port_array.length)
    assert(ha)
    assert(5433, port_array[0])
    assert('test.example.com:5433', host_array[0])
    assert(5434, port_array[1])      
    
    # Pass in other list, localhost, no port
    hostString = 'test.example.com:5433,localhost'
    c = Configure.new(CallResolverConfigure::CSE_HOSTS => hostString)
    config = CallResolverConfigure.new(c)
    
    host_array, port_array, ha = config.get_cse_hosts_config
    
    assert_equal(2, port_array.length)
    assert(ha)
    assert(5433, port_array[0])
    assert(DatabaseUrl::DATABASE_PORT_DEFAULT, port_array[1])
    
    # Pass in other list, localhost, no port
    hostString = 'test.example.com:5433,localhost:6666'
    c = Configure.new(CallResolverConfigure::CSE_HOSTS => hostString)
    config = CallResolverConfigure.new(c)
    
    host_array, port_array, ha = config.get_cse_hosts_config
    
    assert_equal(2, port_array.length)
    assert(ha)
    assert(5433, port_array[0])
    assert(6666, port_array[1])
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
  
  def test_get_agent
    assert_equal('0.0.0.0', @config.agent_address)
    assert_equal(8130, @config.agent_port)
  end  
  
  def test_cse_polling_interval
    assert_equal(10, @config.cse_polling_interval)
  end  
  
end
