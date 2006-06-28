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
require 'pp'
require 'soap/attachment'
require 'soap/rpc/standaloneServer'
require 'soap/mapping'

# application requires
require 'process_manager'


# Turn the ProcessManager into a SOAP server
class ProcessManagerServer < SOAP::RPC::StandaloneServer
  
  DEFAULT_PORT = 8200
  DEFAULT_BIND_ADDRESS = '0.0.0.0'
  SOAP_NAMESPACE = 'urn:ProcessManagerService'
  
  def initialize(process_manager, config = {})
    @process_manager = process_manager
    
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
    add_method(self, 'manageProcesses', 'verb', 'processes')
    add_method(self, 'getProcessStatus')
    add_method(self, 'readFile', 'sipxFilePath')
  end

  def port
    config[:Port]
  end


  #=============================================================================
  # SOAP methods
  
  def manageProcesses(verb, processes)
    # debug printing
    # pretty-print the processes array
    puts("manageProcesses: verb = #{verb}, processes = #{processes}")
    
    @process_manager.manageProcesses(verb, processes)
  end

  #:TODO: pass this call through to the process manager
  def getProcessStatus()
    puts 'getProcessStatus'
    
    # Create and return dummy status info as a start
    s1 = NamedProcessStatus.new
    s1.name = 'proc1'
    s1.status = 'started'
    s2 = NamedProcessStatus.new
    s2.name = 'proc2'
    s2.status = 'stopped'
    return ProcessManagerServer::Array[s1, s2]
  end
  
  #:TODO: implement these methods
  
  def readFile(sipxFilePath)
    puts sipxFilePath.inspect
    dir = @process_manager.get_sipx_directory(sipxFilePath.sipxDir)
    if !File.exists?(dir)
      raise("Directory \"#{dir}\" does not exist")
    end
    path = File.join(dir, sipxFilePath.fileRelativePath)
    if !File.exists?(path)
      raise("File \"#{path}\" does not exist")
    end
    SOAP::Attachment.new(File.open(path))
  end
  
  def writeFile
  end
  def deleteFile
  end
  def readSipData
  end
  def writeSipData
  end
  
  #=============================================================================
  # SOAP data
  
  # marshallable version of the standard Array
  class Array < ::Array; include SOAP::Marshallable
    @@schema_ns = SOAP_NAMESPACE  
  end

  class NamedProcessStatus; include SOAP::Marshallable
    @@schema_ns = SOAP_NAMESPACE
    @@schema_type = 'NamedProcessStatus'
    attr_accessor :name, :status
  end

  class SipxFilePath; include SOAP::Marshallable
    @@schema_ns = SOAP_NAMESPACE
    @@schema_type = 'SipxFilePath'
    attr_accessor :sipxDir, :fileRelativePath
  end
  
end
