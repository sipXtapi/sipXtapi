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
require 'soap/rpc/standaloneServer'
require 'soap/mapping'

# application requires
require 'process_manager'


# Turn the ProcessManager into a SOAP server
class ProcessManagerServer < SOAP::RPC::StandaloneServer
  
  DEFAULT_PORT = 8200
  DEFAULT_BIND_ADDRESS = '0.0.0.0'
  SOAP_NAMESPACE = 'urn:ProcessManagerService'
  
  def initialize(config = {})
    config[:BindAddress] = DEFAULT_BIND_ADDRESS if !config[:BindAddress]
    config[:Port] = DEFAULT_PORT if !config[:Port]

    # This section only works for SOAP4R releases post 1_5_5 (as yet unavailable
    # in released Ruby), but is harmless for earlier releases
    thisdir = File.dirname __FILE__
    config[:WSDLDocumentDirectory] = thisdir    
    config[:SOAPDefaultNamespace] = SOAP_NAMESPACE    

    # config is not used yet - pass explicit parameters
    super('sipXprocessManager', SOAP_NAMESPACE, config[:BindAddress], config[:Port])
  end
  
  def on_init
    @log.level = Logger::Severity::DEBUG
    add_method(self, 'manageProcesses')
  end

  def port
    config[:Port]
  end


  #=============================================================================
  # SOAP methods
  
  def manageProcesses(input)
    puts 'manageProcesses'
    puts input
  end

  
  #=============================================================================
  # SOAP data

  class ManageProcessesInput; include SOAP::Marshallable
    @@schema_ns = SOAP_NAMESPACE
    @@schema_type = 'ManageProcessesInput'
    attr_accessor :processes, :verb
  end
  
  # marshall-able version of the standard Array
  class Array < ::Array; include SOAP::Marshallable
    @@schema_ns = SOAP_NAMESPACE  
  end
  
end
