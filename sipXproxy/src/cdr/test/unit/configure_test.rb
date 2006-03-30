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

# system requires
require 'test_helper'

# application requires
require 'configure'


class ConfigureTest < Test::Unit::TestCase

  def test_config
    # Trying to open a nonexisting config file should fail
    assert_raise(Errno::ENOENT) {Configure.new('nonexistent_config_file')}
    
    # Trying to open a misformatted config file should fail
    assert_raise(ConfigException) {Configure.new(config_file_path('bad_config_missing_colon.txt'))}
    
    # Load a valid config file and check the results
    config = Configure.new(config_file_path('config.txt'))
    assert_equal('sam:/ann', config['SIP_CALLRESOLVER_LOG_DIR'])
    assert_equal("DEBUG", config['SIP_CALLRESOLVER_LOG_LEVEL'])
  end
  
  # Given the name of a config_file in the data directory, return the path
  # to it.
  def config_file_path(config_file)
    File.join($thisdir, "data", config_file)
  end
  
end
