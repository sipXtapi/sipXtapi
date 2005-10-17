require 'test/unit'
require 'soap/wsdlDriver'
require 'xsd/mapping'

SERVICE_ROOT = 'http://localhost:9999/sipxconfig/service'
USER_SERVICE_WSDL = SERVICE_ROOT + '/UserService?wsdl'
TEST_SERVICE_WSDL = SERVICE_ROOT + '/TestService?wsdl'

class UserServiceTestApi < Test::Unit::TestCase

    def setup
		@userService = SOAP::WSDLDriverFactory.new(USER_SERVICE_WSDL).create_rpc_driver
		@userService.wiredump_dev = STDOUT if $DEBUG
		@testService = SOAP::WSDLDriverFactory.new(TEST_SERVICE_WSDL).create_rpc_driver
		@testService.wiredump_dev = STDOUT if $DEBUG
    end

	def test_createUser
    	@testService.clearCoreContext(:parameters => nil)    	
    	userName = 'jtester';
		@userService.createUser(:in0 => userName, 
		        :in1 => '1234', 
                :in2 => 'Joe', 
                :in3 => 'Tester', 
                :in4 => 'joe, the-man', 
                :in5 => '')

        # @userService.findUser(userName)
    end

	def test_deleteUser
    	@testService.clearCoreContext(:parameters => nil)    	
    	userName = 'jtester';
		@userService.createUser(:in0 => userName, 
		        :in1 => '1234', 
                :in2 => 'Joe', 
                :in3 => 'Tester', 
                :in4 => 'joe, the-man', 
                :in5 => '')
		@userService.deleteUser(:in0 => userName)
    end
end