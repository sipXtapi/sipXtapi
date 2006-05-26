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
require 'process_manager'


class ProcessManagerTest < Test::Unit::TestCase
  def setup
    @pm_config_dir = File.join($PROCESS_MANAGER_TEST_DIR, "data", "process")
    @pm = ProcessManager.new(@pm_config_dir)
    @config_file = File.join(@pm_config_dir, 'sleeper.xml')
  end
  
  def test_create_pm
    assert(@pm)
    assert_equal(@pm_config_dir, @pm.send(:config_dir))
    assert(File.exists?(@config_file))
  end
  
  def test_get_process_config_files
    config_files = @pm.send(:get_process_config_files)
    assert_equal(1, config_files.length)
    assert_equal(@config_file, config_files[0])
  end
  
  def test_read_config
    
  end
end
