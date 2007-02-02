#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'time'

require 'db/database_url'
require 'utils/configure'
require 'utils/exceptions'
require 'utils/sipx_logger'


class CallResolverConfigure
  
  # Default config file path
  DEFAULT_CONFIG_FILE = '/etc/sipxpbx/callresolver-config'
  
  # If set, then this becomes a prefix to the default config file path
  SIPX_PREFIX = 'SIPX_PREFIX'
  
  LOCALHOST = 'localhost'
  
  # Max integer in a Fixnum, on a 32-bit machine
  INT_MAX = 2147483647
  
  # Configuration parameters and defaults
  
  # Whether console logging is enabled or disabled.  Legal values are "ENABLE"
  # or "DISABLE".  Comparison is case-insensitive with this and other values.
  LOG_CONSOLE_CONFIG = 'SIP_CALLRESOLVER_LOG_CONSOLE'
  LOG_CONSOLE_CONFIG_DEFAULT = Configure::DISABLE
  
  # The directory holding log files.  The default value is prefixed by
  # $SIPX_PREFIX if that environment variable is defined.
  LOG_DIR_CONFIG = 'SIP_CALLRESOLVER_LOG_DIR'
  LOG_DIR_CONFIG_DEFAULT = '/var/log/sipxpbx'
  
  # Logging severity level
  LOG_LEVEL_CONFIG = 'SIP_CALLRESOLVER_LOG_LEVEL'
  LOG_LEVEL_CONFIG_DEFAULT = 'NOTICE'
  
  LOG_FILE_NAME = 'sipcallresolver.log'
  
  DAILY_RUN = 'SIP_CALLRESOLVER_DAILY_RUN'
  DAILY_RUN_DEFAULT = Configure::DISABLE
  
  PURGE = 'SIP_CALLRESOLVER_PURGE'
  PURGE_DEFAULT = Configure::ENABLE
  
  PURGE_AGE_CDR = 'SIP_CALLRESOLVER_PURGE_AGE_CDR'
  PURGE_AGE_CDR_DEFAULT = 35
  
  PURGE_AGE_CSE = 'SIP_CALLRESOLVER_PURGE_AGE_CSE'
  PURGE_AGE_CSE_DEFAULT = 7
  
  CSE_HOSTS = 'SIP_CALLRESOLVER_CSE_HOSTS'
  CSE_HOSTS_DEFAULT = "#{LOCALHOST}:#{DatabaseUrl::DATABASE_PORT_DEFAULT}"
  
  # Specify this string as the config_file to get a completely default config 
  DEFAULT_CONFIG = 'default_config'
  
  class << self
    def from_file(config_file = nil)
      # If the config_file arg is nil, then find the config file in the default location
      config_file = apply_config_file_default(config_file)
      
      # Set up a generic Configure object that just knows how to parse a config
      # file with param:value lines.
      configure = if config_file == DEFAULT_CONFIG
        Configure.new()
      else
        if File.exists?(config_file)
          $stderr.puts("Reading config from #{config_file}")        
          Configure.from_file(config_file)
        else
          $stderr.puts("Config file #{config_file} not found, using default settings")
          Configure.new()
        end
      end
      
      CallResolverConfigure.new(configure)
    end
    
    # Given a config_file name, if it is non-nil then just return it.
    # If it's nil then return the default config file path, prepending
    # $SIPX_PREFIX if that has been set.
    def apply_config_file_default(config_file)
      return config_file if config_file
      
      prefix = ENV[SIPX_PREFIX]
      if prefix 
        return File.join(prefix, DEFAULT_CONFIG_FILE)
      else
        return DEFAULT_CONFIG_FILE
      end                  
    end
    
  end
  
  def initialize(config)  
    @config = config
    
    # Read logging config and initialize logging.  Do this before initializing
    # the rest of the config so we can use logging there.
    @log = init_logging
    
    # Finish setting up the config
    finish_config
  end
  
  #-----------------------------------------------------------------------------
  # Public configuration
  
  OTHER_PARAMS = [:cdr_database_url, :cse_database_urls,
  :host_list, :host_port_list, :log]
  # Define a reader for each of the other params
  attr_reader(*OTHER_PARAMS)
  
  # Return true if High Availability (HA) is enabled, false otherwise
  def ha?
    @ha
  end
  
  # Return true if database purging is enabled, false otherwise
  def purge?
    @config.enabled?(PURGE, PURGE_DEFAULT)
  end

  # Compute start time of CDR records to be purged from configuration
  def purge_age_cdr
    return nil unless purge?
    return parse_int_param(@config, PURGE_AGE_CDR, PURGE_AGE_CDR_DEFAULT, 1)
  end    
  
  # Compute start time of CSE records to be purged from configuration
  def purge_age_cse
    return nil unless purge?
    return parse_int_param(@config, PURGE_AGE_CSE, PURGE_AGE_CSE_DEFAULT, 1)
  end
  
  def agent_port
    @config.fetch('SIP_CALLRESOLVER_AGENT_PORT', 8130)
  end
  
  def agent_address
    @config.fetch('SIP_CALLRESOLVER_AGENT_ADDR', '0.0.0.0')
  end
  
  # Access the config as an array.  Use this method *only* for plugin config
  # params that are unknown to the call resolver.  All known params should be
  # retrieved using the above accessors.
  def [](param)
    @config[param]
  end
  
  def enabled?(param, default = nil)
    @config.enabled?(param, default)
  end
  
  # Set up logging.  Return the Logger.
  def init_logging    
    # If console logging was specified, then do that.  Otherwise log to a file.
    if @config.enabled?(LOG_CONSOLE_CONFIG, LOG_CONSOLE_CONFIG_DEFAULT)
      log_device = STDOUT
    else
      log_dir = get_log_dir_config
      if File.exists?(log_dir)
        log_device = File.join(log_dir, LOG_FILE_NAME)
        # If the file exists, then it must be writable. If it doesn't exist,
        # then the directory must be writable.
        if File.exists?(log_device)
          if !File.writable?(log_device)
            $stderr.puts("init_logging: Log file '#{log_file}' exists but is not writable. " +
                 "Log messages will go to the console.") if $DEBUG 
            log_device = STDOUT
          end
        else
          if !File.writable?(log_dir)
            $stderr.puts("init_logging: Log directory '#{@log_dir}' is not writable. " +
                 "Log messages will go to the console.") if $DEBUG 
            log_device = STDOUT
          end
        end
      else
        $stderr.puts("Unable to open log file, log directory '#{@log_dir}' does not " +
             "exist.  Log messages will go to the console.") if $DEBUG 
        log_device = STDOUT
      end
    end
    log = SipxLogger.new(log_device)
    
    # Set the log level from the configuration
    log.level = get_log_level_config(@config)
    
    # Override the log level to DEBUG if $DEBUG is set.
    # :TODO: figure out why this isn't working.
    if $DEBUG then
      log.level = Logger::DEBUG
    end
    
    return log
  end
  
  # Set the log directory from the configuration.  Return the log directory.
  def get_log_dir_config
    # Look up the config param
    log_dir = @config[LOG_DIR_CONFIG]
    return log_dir if log_dir
    
    # Prepend the prefix dir if $SIPX_PREFIX is defined
    prefix = ENV[SIPX_PREFIX]
    if prefix
      File.join(prefix, LOG_DIR_CONFIG_DEFAULT)
    else
      LOG_DIR_CONFIG_DEFAULT
    end
  end
  
  # Set the log level from the configuration.  Return the log level.
  def get_log_level_config(config)
    # Look up the config param
    log_level_name = config[LOG_LEVEL_CONFIG] || LOG_LEVEL_CONFIG_DEFAULT
    
    # Convert the log level name to a Logger log level
    SipxLogger::LOG_LEVEL_SIPX_TO_LOGGER.fetch(log_level_name) { | name |
      # If we don't recognize the name, then refuse to run.  Would be nice to
      # log a warning and continue, but there is no log yet!    
      raise CallResolverException, "Unknown log level: #{name}"
    } 
  end
  
  #-----------------------------------------------------------------------------
  
  # Finish setting up the config.  Logging has already been set up before this,
  # so we can log messages in the methods that are called here.
  # Read config params, applying defaults
  def finish_config    
    # :TODO: read CDR database URL params from the Call Resolver config file
    # rather than just hardwiring default values.
    @cdr_database_url = DatabaseUrl.new
    
    # These two methods must get called in this order
    @host_list, @host_port_list, @ha = get_cse_hosts_config
    @cse_database_urls = get_cse_database_urls_config(@host_port_list)
  end
  
  
  # Return an array of CSE database URLs.  With an HA configuration, there are
  # multiple CSE databases.  Note that usually one of these URLs is identical
  # to the CDR database URL, since a standard master server runs both the
  # proxies and the call resolver, which share the SIPXCDR database.
  def get_cse_database_urls_config(host_port_list)
    if host_port_list.empty?
      [ cdr_database_url ] 
    else      
      # Build the list of CSE DB URLs.  From Call Resolver's point of view,
      # each URL is 'localhost:<port>'.  Stunnel takes care of forwarding the
      # local port to the database on a remote host.
      host_port_list.collect do |port|
        DatabaseUrl.new(:port => port)
      end
    end     
  end
  
  # Get distributed CSE hosts from the configuration. 
  # 
  # Return two arrays host_list, host_port_list
  # 
  # Call resolver connects to each of these ports on 'localhost' via the
  # magic of stunnel, so it doesn't ever use the hostnames.
  def get_cse_hosts_config
    host_list = @config[CSE_HOSTS] || CSE_HOSTS_DEFAULT
    
    host_port_list = []
    ha = false
    # Split host list into separate host:port names, then build two
    # arrays of URLs and ports.
    host_array = host_list.split(',')
    host_array.each do |host_string|
      host, port = host_string.split(':')
      # Strip leading and trailing whitespace
      host.strip!
      # Test if port was specified      
      if port
        # Strip whitespace from port
        port.strip!        
      else
        # Supply default port for localhost
        if host == LOCALHOST
          port = DatabaseUrl::DATABASE_PORT_DEFAULT
        else
          raise ConfigException, "No port specified for host '#{host}'. " +
            "A port number for hosts other than  'localhost' must be specified."
        end
      end
      host_port = port.to_i
      if host_port == 0
        raise ConfigException, "Port for #{host} is invalid."
      end
      host_port_list << host_port
      log.debug("set_cse_hosts_config: host name #{host}, host port: #{port}")
      # If at least one of the hosts != 'localhost' we are HA enabled
      if host != 'localhost' && !ha
        ha = true
        log.debug("get_cse_host: Found host other than localhost - enable HA")
      end
    end
    return host_array, host_port_list, ha
  end  
  
  # Read the named param from the config.  Convert it to an integer and return
  # the value.  If the param is not defined, then use the default.  Validate
  # that the param is between the min and max and raise a ConfigException if not.
  def parse_int_param(config, param_name, default_value = nil, min = 0, max = INT_MAX)
    param_value = default_value
    begin
      value = config[param_name]
      param_value = Integer(value) if value
      if !(min..max).include?(param_value)
        raise ConfigException, "Configuration parameter #{param_name}, #{param_value} must be in #{min}..#{max} range."
      end
    rescue ArgumentError
      raise ConfigException, "Configuration parameter #{param_name}, #{param_value} must be an integer"
    end
    
    param_value
  end
end
