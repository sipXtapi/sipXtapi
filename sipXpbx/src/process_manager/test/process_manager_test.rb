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
require 'tmpdir'
require 'test/unit'

# application requires
require 'process_manager'


class ProcessManagerTest < Test::Unit::TestCase
  TEST_PROCESS_NAME = 'sleeper'
  TEST_PROCESS_RUN_PARAMS = "999"
  
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
  
  def test_init_process_config
    config_map = @pm.send(:config_map)
    assert(config_map)
    assert_equal(1, config_map.size)
    config = config_map[TEST_PROCESS_NAME]
    assert(config)
    assert_equal(TEST_PROCESS_NAME, config.name)
    assert_equal(TEST_PROCESS_RUN_PARAMS, config.run.parameters)
  end
  
  def test_get_process_config_files
    config_files = @pm.send(:get_process_config_files)
    assert_equal(1, config_files.length)
    assert_equal(@config_file, config_files[0])
  end
  
  def test_start_process
    config = ProcessConfig.new
    
    # Run the echo command.  Print out "moose" with no newline.
    run = ProcessConfig::Run.new("echo",              # command
                                 "-n moose")          # parameters
    config.run = run
    @pm.send(:start_process, config)
    
    # Create a temporary file to check that the defaultdir setting is right
    tmpdir = Dir.tmpdir
    if tmpdir   
      tempfilename = 'process_manager_test_tempfile'
      tempfilepath = File.join(tmpdir, tempfilename)
      
      # Delete the temp file if it already exists so we can do a clean test
      if File.exists?(tempfilepath)
        File.delete(tempfilepath)
      end
      
      # Run the command and verify that it created the file
      run = ProcessConfig::Run.new("touch",           # command
                                   tempfilename,      # parameters
                                   tmpdir)            # defaultdir
      config.run = run
      assert("File \"#{tempfilepath}\" must not exist but does", !File.exists?(tempfilepath))
      @pm.send(:start_process, config)
      assert("File \"#{tempfilepath}\" must exist but doesn't", File.exists?(tempfilepath))
      if File.exists?(tempfilepath)  # if I don't check this then I get an error on deletion?
        File.delete(tempfilepath)
      end
    else
      puts("test_start_process: there is no temporary directory to write into, can't run the test")
    end
  end
end
