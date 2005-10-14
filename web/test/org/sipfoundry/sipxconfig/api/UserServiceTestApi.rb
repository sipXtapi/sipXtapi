require 'test/unit'
require 'soap/wsdlDriver'

SERVICE_WSDL = 'http://localhost:9999/sipxconfig/service/UserService?wsdl'

class UserServiceTestApi < Test::Unit::TestCase

    @userService    

    def setup
		@userService = SOAP::WSDLDriverFactory.new(SERVICE_WSDL).create_rpc_driver
		@userService.wiredump_dev = STDOUT
    end

	def test_getUserProperties
		properties = @userService.getUserProperties()	
		puts "the userService returned: #{properties}" 	
    end

	def test_createUser
		@userService.createUser(:in0 => 'testApiUser', :in1=> '1234')	
		@userService.deleteUser('testApiUser')
    end
end