#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# set up the load path
$PROCESS_MANAGER_TEST_DIR = File.dirname(__FILE__)
$:.unshift File.join($PROCESS_MANAGER_TEST_DIR, "..", "lib")

# system requires
require 'test/unit'

# application requires
require 'process_config'


class ProcessConfigTest < Test::Unit::TestCase
  def setup
    # Create a ProcessConfig that hasn't read its config yet
    @config = ProcessConfig.new

    @pm_config_dir = File.join($PROCESS_MANAGER_TEST_DIR, "data", "process")
    @config_file = File.join(@pm_config_dir, 'sleeper.xml')
  end
  
  def test_read_config
    assert(File.exists?(@config_file))
    @config.send(:read_config, File.new(@config_file))
    
    # Check the process name
    assert_equal('sleeper', @config.name)
    
    # Check the run info
    run = @config.run
    assert(run)
    assert_equal("/bin/sleep", run.command)
    assert_equal("999", run.parameters)
    assert_equal("/var/log/sipxpbx", run.defaultdir)
  end
end
