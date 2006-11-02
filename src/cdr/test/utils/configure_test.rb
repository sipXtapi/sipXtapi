#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'test/unit'

$:.unshift File.join(File.dirname(__FILE__), '..', '..', 'lib')

require 'utils/configure'

$:.unshift File.join(File.dirname(__FILE__), "..", "..", "test")
require 'test_util'
include TestUtil


class ConfigureTest < Test::Unit::TestCase

  include TestUtil
  
  def test_missing
    # Trying to open a nonexisting config file should fail
    assert_raise(Errno::ENOENT) do
      Configure.new('nonexistent_config_file', NullLog.new)
    end
  end
  
  
  def test_bad_format
    # Trying to open a misformatted config file should fail
    assert_raise(ConfigException) do
      file = config_file_path('bad_config_missing_colon.txt')
      Configure.new(file, NullLog.new)
    end
  end
  
  def test_config
    # Load a valid config file and check the results
    config = Configure.new(config_file_path('config.txt'), NullLog.new)
    assert_equal('sam:/ann', config['SIP_CALLRESOLVER_LOG_DIR'])
    assert_equal('DEBUG', config['SIP_CALLRESOLVER_LOG_LEVEL'])
    assert_equal('3:00', config['SIP_CALLRESOLVER_DAILY_START_TIME'])
    assert_nil(config['EMPTY_PARAM'], 'Params that are in the file but empty must have nil values')
    assert_nil(config['NOT_THERE_PARAM'], 'Params that are not in the file must have nil values')
  end

  def test_enabled?
    # Load a valid config file and check the results
    config = Configure.new(config_file_path('config.txt'), NullLog.new)
    assert_raise(ConfigException) do 
      config.enabled?('SIP_CALLRESOLVER_LOG_DIR')
    end      
    assert(config.enabled?('ABC'))
    assert(!config.enabled?('DEF'), 'ENABLE')
    assert(config.enabled?('NOT_THERE_PARAM', 'ENABLE'))
  end
  
  # Given the name of a config_file in the data directory, return the path
  # to it.
  def config_file_path(config_file)
    File.join(File.dirname(__FILE__), '..', 'data', config_file)
  end  
end
