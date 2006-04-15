require 'server'

def main(port)
  # create the SOAP server
  server = StatsServer.new(:Port => port)
  trap(:INT) do 
    $stderr.puts "shutting down"
    server.shutdown
  end
  
  # start the server
  $stderr.puts "starting server on port #{port}"  
  server.start
end

if __FILE__ == $0
  # TODO: at the moment one can pass parameters through environment variables, it may be more convenient to do it through ARGV
  SIPXSTATS_PREFIX = ENV['SIPXSTATS_PREFIX'] || ''
  SIPXSTATS_SOAP_PORT = ENV['SIPXSTATS_SOAP_PORT'] || 2000  
  main(SIPXSTATS_SOAP_PORT)  
end