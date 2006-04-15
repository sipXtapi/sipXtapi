$:.unshift File.join(File.dirname(__FILE__), "..", "lib")
require 'test/unit'
require 'net/http'
require 'server'
require 'soap/rpc/driver'
require 'main'

class ServerTest < Test::Unit::TestCase

  def start_server

    # not starting eachtime, port doesn't get freed intime
    return if defined? @@server
    puts "starting"

    httpd = StatsServer.new(:Port => 2000)

    trap(:INT) do
      httpd.shutdown
    end

    @@server = Thread.new do
      httpd.start
    end
    puts "started"    
  end
  
  def setup
    super
    start_server
    @stats = SOAP::RPC::Driver.new('http://localhost:2000/', 'urn:StatsService')
    @stats.wiredump_dev = STDERR if $DEBUG
    @stats.add_method('getBirdArray')
  end
  
  def teardown
  end
    
  # fails SOAP4R in w/1.8.4i
  def _test_get_wsdl
    response = Net::HTTP.get_response(URI.parse('http://localhost:2000/wsdl/sipxstats.wsdl'))
    assert_equal('200', response.code)
  end
  
  def test_get_birds
    actual = @stats.getBirdArray    
    assert_equal("robin", actual[0].species)
    assert_equal("bluejay", actual[1].species)
  end
end
