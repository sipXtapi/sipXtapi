#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# Application requires.  Assume that the load path has been set up for us.
require 'call_resolver_configure'
require 'configure'
require 'database_url'
require 'exceptions'
require 'utils'

# StunnelConnection attempts to open stunnel connections to all configured
# distributed machines.
class StunnelConnection

  # Constants
  LOCALHOST = 'localhost'
  SIPX_PREFIX = 'SIPX_PREFIX'
  
  STUNNEL_CONFIG_FILE = 'stunnel-config.tmp'
  STUNNEL_EXEC = '/usr/sbin/stunnel'
 
  CSE_STUNNEL_DEBUG_LEVEL = 'SIP_CALLRESOLVER_STUNNEL_DEBUG'
  
  # Default debug level NOTICE  
  CSE_STUNNEL_DEBUG_LEVEL_DEFAULT = '5'
  CSE_CONNECT_PORT = '9300'
  
  # No default for the CA file name - if HA is configured this has to be set
  CSE_CA = 'SIP_CALLRESOLVER_CSE_CA'
  
  SIGNAL_SIGQUIT = 3
  SIPXPBX_SSLPATH = 'etc/sipxpbx/ssl'
  
public

  def initialize(resolver)
    @resolver = resolver
    @connection_established = false
  end

  def open(config)
    # Look up the config param and generate the stunnel config file.
    # Set the @ha_enabled variable depending on if any distributed
    # machines were configured.
    get_stunnel_config(config)
    
    # Open stunnel connection if HA is enabled
    if @ha_enabled
      if ! check_stunnel_running
        fork do exec("#{STUNNEL_EXEC} #{STUNNEL_CONFIG_FILE}") end
        # Give stunnel time to start up
        sleep 1
        # Get the Pid of the process we just started (stored as instance variable),
        # also check if it really started
        if ! check_stunnel_running
          Utils.raise_exception("stunnel could not be started.")
        else
          @connection_established = true
        end
      else
        Utils.raise_exception("stunnel is already running with Pid #{@pid}. It must be shut " +
              " down before restarting the call resolver.")
      end
    end
  end
  
  def close
    # Only do something if connection was established
    if @connection_established
      log.debug("StunnelConnection.close: Killing pid #{@pid}")
      Process.kill(SIGNAL_SIGQUIT, @pid.to_i)
      File.delete("#{STUNNEL_CONFIG_FILE}")
    end
  end

private

  # Get possible distributed CSE hosts from configuration file. Generate
  # an stunnel configuration script and return an array of ports.
  def get_stunnel_config(config)
    host_list = config.host_list
    host_url_list = []
    host_port_list = []
    @ha_enabled = false
    # Split host list into separate host:port names, then build two
    # arrays of URLs and ports.
    host_array = host_list.split(',')
    host_array.each do |host_string|
      host_elements = host_string.split(':')
      # Strip leading and trailing whitespace
      host_elements[0] = host_elements[0].strip
      # Test if port was specified      
      if host_elements.length == 1
        # Supply default port for localhost
        if host_elements[0] == LOCALHOST
          host_elements[1] = DatabaseUrl::DATABASE_PORT_DEFAULT.to_s
          puts "i\'m here" 
        else
          Utils.raise_exception(
            "No port specified for host \"#{host_elements[0]}\". " +
            "A port number for hosts other than  \"localhost\" must be specified.")
        end
      else
        # Strip whitespace from port
        host_elements[1] = host_elements[1].strip
      end
      host_url_list << host_elements[0]
      host_port_list << host_elements[1]
      log.debug("get_stunnel_config: host name #{host_elements[0]}, host port: #{host_elements[1]}")
      # If at least one of the hosts != 'localhost' we are HA enabled
      if host_elements[0] != 'localhost' && ! @ha_enabled
        @ha_enabled = true
        log.debug("get_stunnel_config: Found host other than localhost - enable HA")
      end
    end
    # Check if we are HA enabled
    if @ha_enabled
      # get the name of the CA file - no defaults here, must be specified
      ca_file = config[CSE_CA]
      err_msg = "No CA file name specified. If hosts other than \"localhost\" " +
                "are specified in #{CSE_HOSTS} then this parameter must be set."
      if ca_file == nil
        Utils.raise_exception(err_msg)
      else
        ca_file = ca_file.strip       
        if ca_file.length == 0
          Utils.raise_exception(err_msg)        
        end
      end
      debug_level = config[CSE_STUNNEL_DEBUG_LEVEL]
      debug_level ||= CSE_STUNNEL_DEBUG_LEVEL_DEFAULT      
      generate_stunnel_config(host_url_list, host_port_list, ca_file, debug_level)
    end
  end
  
  # Generate the stunnel configuration based on the call resolver configuration  
  def generate_stunnel_config(host_list, port_list, ca_file, debug_level)
    prefix = ENV[SIPX_PREFIX]
    
    config_file = File.new("#{STUNNEL_CONFIG_FILE}",  "w")
    log.debug("Master machine stunnel configuration:")
    config_file.puts "# This file was generated by call_resolver.rb"
    config_file.puts "client = yes"
    config_file.puts "CAfile = #{prefix}/#{SIPXPBX_SSLPATH}/authorities/#{ca_file}"
    config_file.puts "cert = #{prefix}/#{SIPXPBX_SSLPATH}/ssl.crt"
    config_file.puts "key = #{prefix}/#{SIPXPBX_SSLPATH}/ssl.key"
    config_file.puts "verify = 2"
    config_file.puts "debug = #{debug_level}"
    config_file.puts "output = #{prefix}/var/log/sipxpbx/stunnel.log"
    
    log.debug("CAfile = #{prefix}/#{SIPXPBX_SSLPATH}/authorities/#{ca_file}")    
    log.debug("cert = #{prefix}/#{SIPXPBX_SSLPATH}/ssl.crt")
    log.debug("key = #{prefix}/#{SIPXPBX_SSLPATH}/ssl.key")
    log.debug("debug = #{debug_level}")
    
    host_list.each_with_index do |host, i|
      # Don't generate entry for localhost
      if host_list[i] != LOCALHOST
        config_file.puts ""
        config_file.puts "[Postgres-#{i}]"
        config_file.puts "accept = #{port_list[i]}"
        config_file.puts "connect = #{host}:#{CSE_CONNECT_PORT}"
        log.debug("accept = #{port_list[i]}")
        log.debug("connect = #{host}:#{CSE_CONNECT_PORT}")
      end
    end
    config_file.close()
  end
  
  def check_stunnel_running()
    shellReturn = `ps -fC stunnel | grep #{STUNNEL_EXEC}`
    shellReturn = shellReturn.strip
    if /\A\D+\s+(\d+)\s+.*stunnel.*/ =~ shellReturn
      @pid = $1
      running = true
    else
      @pid = '0'
      running = false
    end
    running
  end

  # Use the Call Resolver's Logger
  def log
    @resolver.log
  end
  
end
