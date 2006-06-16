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
  TEST_PROCESS_RUN_PARAMS = "10"
  
  def setup
    @process_config_dir = File.join($PROCESS_MANAGER_TEST_DIR, "data", "process")
    @pm = ProcessManager.new(:ProcessConfigDir => @process_config_dir)
    @config_file = File.join(@process_config_dir, 'sleeper.xml')
    @tmpdir = Dir.tmpdir
    assert(@tmpdir, "There is no tmpdir, can't run tests")
  end
  
  def test_create_pm
    assert(@pm)
    assert_equal(@process_config_dir, @pm.send(:process_config_dir))
    assert(File.exists?(@config_file))
  end
  
  def test_init_process_config
    process_config_map = @pm.send(:process_config_map)
    assert(process_config_map)
    assert_equal(1, process_config_map.size)
    config = process_config_map[TEST_PROCESS_NAME]
    assert(config)
    assert_equal(TEST_PROCESS_NAME, config.name)
    assert_equal(TEST_PROCESS_RUN_PARAMS, config.run.parameters)
  end
  
  def test_get_process_config_files
    config_files = @pm.send(:get_process_config_files)
    assert_equal(1, config_files.length)
    assert_equal(@config_file, config_files[0])
  end
  
  def test_init_pid_dir
    # If SIPX_PREFIX is not set, then the pid_dir must be the default
    ENV[ProcessManager::SIPX_PREFIX] = nil
    @pm.send(:init_pid_dir)
    assert_equal(ProcessManager::PID_DIR_DEFAULT, @pm.send(:pid_dir))
    
    # If SIPX_PREFIX is set, then the pid_dir must be the default with
    # SIPX_PREFIX prepended
    prefix = '/tmp'
    ENV[ProcessManager::SIPX_PREFIX] = prefix
    @pm.send(:init_pid_dir)
    assert_equal(File.join(prefix, ProcessManager::PID_DIR_DEFAULT), @pm.send(:pid_dir))    
  end
  
  def test_start_process
    config = ProcessConfig.new

    # Change the PID dir to be temporary, for testing
    @pm.send(:pid_dir=, @tmpdir)
  
    # Run the echo command.  Print out "moose" with no newline.
    config.name = 'moose'
    run = ProcessConfig::Run.new("echo",              # command
                               "-n moose")          # parameters
    config.run = run
    @pm.send(:start_process, config)
  
    # Create a temporary file to check that the defaultdir setting is right
    tempfilename = 'process_manager_test_tempfile'
    tempfilepath = File.join(@tmpdir, tempfilename)
    
    # Delete the temp file if it already exists so we can do a clean test
    if File.exists?(tempfilepath)
      File.delete(tempfilepath)
    end
    
    # Run the command
    run = ProcessConfig::Run.new("touch",           # command
                                 tempfilename,      # parameters
                                 @tmpdir)           # defaultdir
    config.run = run
    assert(!File.exists?(tempfilepath), "File \"#{tempfilepath}\" must not exist but does")
    @pm.send(:start_process, config)
    
    # Give the child process some time to create the file
    1.upto(100) do |i|
      if File.exists?(tempfilepath)
        break
      end
      sleep 0.01                                    # sleep for 1/100 of a second
    end
    
    # Verify that it created the file
    assert(File.exists?(tempfilepath), "File \"#{tempfilepath}\" must exist but doesn't")
    
    # Clean up
    File.delete(tempfilepath)
  end

  def test_create_process_pid_file
    process_name = 'extradition'
    pid = 1959
    
    # Change the PID dir to be temporary, for testing
    @pm.send(:pid_dir=, @tmpdir)
    
    # Make sure the PID file doesn't already exist, for a clean test
    pid_file_path = File.join(@tmpdir, process_name + ProcessManager::PID_FILE_EXT)
    if File.exists?(pid_file_path)
      File.delete(pid_file_path)
    end
    assert(!File.exists?(pid_file_path), "File \"#{pid_file_path}\" must not exist but does")
    
    # Create the PID file
    path = @pm.send(:create_process_pid_file, process_name, pid)
    assert_equal(pid_file_path, path, "Expected PID file path to be \"#{pid_file_path}\" but was \"#{path}\"")

    # Check that it worked
    assert(File.exists?(pid_file_path), "File \"#{pid_file_path}\" must exist but doesn't")
    File.open(pid_file_path) do |file|
      saved_pid = file.readline
      assert_equal(pid, saved_pid.to_i)
    end
    
    # Clean up
    File.delete(pid_file_path)
  end
end
