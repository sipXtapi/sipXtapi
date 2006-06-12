#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# system requires
require 'logger'

# application requires
require 'process_config'

=begin
:TODO: Start, stop, report status
:TODO: Don't start a process if it's already running
:TODO: Monitor processes and try to restart them if they go down

:TODO: Honor process config:
:TODO: "manage" flag -- if false then don't do anything with the process
:TODO: "max_restarts" setting
:TODO: "report", "max_reports" settings
:TODO: "failure_contact" setting
:TODO: "stdout", "stderr", "stdin" settings

:TODO: Remote method to replicate files
:TODO: Remote method to replicate sipdb data
:LATER: Remote method to add/edit/remove crontab entry
:LATER: Create the process schema referenced by process config files
=end

# ProcessManager features:
# * Start, monitor, stop, report status on sipX processes
#   * Replaces watchdog, processcgi, WatchDog.xml, ProcessDefinitions.xml, ...
# * Offer network services:
#   * Read/write sipdb databases and arbitrary files (replace and extend replicationcgi)
# * Configured via files that are installed into /etc/sipxpbx/process
class ProcessManager
  
  # If set, then this becomes a prefix to the default sipX directories
  SIPX_PREFIX = 'SIPX_PREFIX'

  # Default directory in which to store PID files for the processes we are managing
  PID_DIR_DEFAULT = '/var/run/sipxpbx'
  
  # Default directory in which process config files are located
  PROCESS_CONFIG_DIR_DEFAULT = '/etc/sipxpbx/process'
  
  PID_FILE_EXT = '.pid'
  
  CONFIG_FILE_PATTERN = Regexp.new('.*\.xml')
  
public

  def initialize(config = {})
    init_process_config_dir(config[:ProcessConfigDir])
    init_process_config
    init_logging
    init_pid_dir
  end

  attr_accessor :pid_dir
  attr_reader :process_config_dir, :process_config_map, :log

private

  def init_process_config_dir(process_config_dir)
    if process_config_dir
      @process_config_dir = process_config_dir
    else
      @process_config_dir = PROCESS_CONFIG_DIR_DEFAULT
      
      # Prepend the prefix dir if $SIPX_PREFIX is defined
      prefix = ENV[SIPX_PREFIX]
      if prefix
        @process_config_dir = File.join(prefix, @process_config_dir)
      end
    end
  end
  
  # Each config file in the config dir sets up a sipX process.
  # Read config info from those files and build a process map.
  def init_process_config
    @process_config_map = {}
    get_process_config_files.each do |file|
      config = ProcessConfig.new(File.new(file))
      @process_config_map[config.name] = config
    end
    @process_config_map
  end
  
  # Return an array containing the paths of process config files.
  def get_process_config_files
    config_files = []
    Dir.foreach(@process_config_dir) do |file|
      config_files << File.join(@process_config_dir, file) if file =~ CONFIG_FILE_PATTERN
    end
    config_files
  end
  
  def init_logging
    @log = Logger.new(STDOUT)
    @log.level = Logger::DEBUG
  end
  
  # Set @pid_dir to be the directory in which to store PID files for the
  # processes we are managing.
  def init_pid_dir
    @pid_dir = PID_DIR_DEFAULT
      
    # Prepend the prefix dir if $SIPX_PREFIX is defined
    prefix = ENV[SIPX_PREFIX]
    if prefix
      @pid_dir = File.join(prefix, @pid_dir)
    end
  end

  # Start the named process. Raise an exception if no such process is configured.
  def start_process_by_name(process_name)
    config = @process_config_map[process_name]
    if !config
      raise("Cannot start \"#{process_name}\", no such process is configured")
    end
    start_process(config)
  end

  # Start the named process. Raise an exception if no such process is configured.
  def start_process(process_config)
    # Get info on how to run the process. Assume that the config has been validated already.
    run = process_config.run
    command = run.command
    parameters = run.parameters
    defaultdir = run.defaultdir
    
    # Start the process
    pid = fork do
      log.debug("start_process: command = \"#{command}\", parameters = " +
                "\"#{parameters}\", defaultdir = \"#{defaultdir}\"")
      Dir.chdir(defaultdir) if defaultdir
      exec("#{command} #{parameters}")
    end

    # Remember the process
    pid_file_path = create_process_pid_file(process_config.name, pid)
    log.debug("start_process: PID file = \"#{pid_file_path}\"")
  end

  # Create a PID file for the named process.  Return the path to the file.
  def create_process_pid_file(process_name, pid)
    pid_file_path = File.join(@pid_dir, process_name + PID_FILE_EXT)
    File.open(pid_file_path, 'w') do |file|
      file.puts("#{pid}")
    end
    
    pid_file_path
  end

end




