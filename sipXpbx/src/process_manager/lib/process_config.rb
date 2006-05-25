#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# system requires
require 'rexml/document'
include REXML  # so that we don't have to prefix everything with REXML::...

# application requires


# The ProcessConfig class holds configuration info for a process managed by ProcessManager
class ProcessConfig

public

  # Constants
  
  # XML element names
  PROCESS_ELEMENT_NAME = 'process'
  RUN_ELEMENT_NAME = 'run'
  
  # XML attribute names
  NAME_ATTRIB = 'name'

  # The Run class holds the info needed to run a process
  class Run
    attr_accessor :command, :parameters, :defaultdir
    def initialize(command, parameters, defaultdir)
      @command = command
      @parameters = parameters
      @defaultdir = defaultdir
    end
  end

  def initialize(config_file = nil)
    if config_file
      read_config(config_file)
    end
  end

  attr_accessor :name,        # name identifying the process
                :run          # object describing how to run the process

private

  # Read the config and init data members.
  def read_config(config_file)
    doc = Document.new(config_file)
    root = doc.root
    
    # Read the process name
    @name = root.attributes[NAME_ATTRIB]
    unless @name
      raise("Process config #{config_file} does not declare a process name")
    end
    
    # Read info on how to run the process.  There should only be one 'run' element.
    @run = nil
    doc.elements.each('//run') do |element|
      command = element.attributes['command']
      parameters = element.attributes['parameters']
      defaultdir = element.attributes['defaultdir']
      if @run
        raise("Process config #{config_file} contains multiple 'run' elements")
      end
      @run = Run.new(command, parameters, defaultdir)
    end
  end

end
