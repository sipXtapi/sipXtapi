require 'test/unit'
require 'soap/wsdlDriver'

SERVICE_WSDL = 'http://localhost:9999/sipxconfig/service/UserService?wsdl'

class UserServiceTestApi < Test::Unit::TestCase

    @userService    

    def setup
		@userService = SOAP::WSDLDriverFactory.new(SERVICE_WSDL).create_rpc_driver
		@userService.wiredump_dev = STDOUT
    end

	def test_echo
		result = @userService.createUser(:in0 => 'testApiUser', :in1=> '1234')	
		puts "the userService returned: #{result}" 	
    end
end