require 'soap/rpc/standaloneServer'
require 'soap/mapping'

StatsServerNamespace = 'urn:StatsService'
class StatsServer < SOAP::RPC::StandaloneServer
  def initialize(config)
    config[:BindAddress] = '0.0.0.0'

    # this section only works for releases post 1_5_5 (as yet unavailable in released ruby)
    # but harmless for earlier releases
    thisdir = File.dirname __FILE__
    config[:WSDLDocumentDirectory] = thisdir    
    config[:SOAPDefaultNamespace] = StatsServerNamespace
    
    # config is not used yet - pass explicit parameters
    super('sipXconfigAgent', StatsServerNamespace, config[:BindAddress], config[:Port])
  end
  
  def on_init
    #@log.level = Logger::Severity::DEBUG
    add_method(self, 'getBirdArray')
  end
  
  def getBirdArray()    
    robin = Bird.new
    robin.species = "robin"
    bluejay = Bird.new
    bluejay.species = "bluejay"
    return BirdArray[robin, bluejay]
  end
  
end

class Bird; include SOAP::Marshallable
  @@schema_ns = StatsServerNamespace
  attr_accessor :species
end

class BirdArray < Array; include SOAP::Marshallable
  @@schema_ns = StatsServerNamespace
end

if $0 == __FILE__
  # this is just a test - pass empty acd server here
  server = StatsServer.new(:Port => 2000)
  trap(:INT) do 
    puts "shutting down"
    server.shutdown
  end
  puts "starting server on port 2000"
  server.start
end
