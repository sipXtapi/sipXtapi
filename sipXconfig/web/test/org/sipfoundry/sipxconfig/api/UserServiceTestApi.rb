require 'test/unit'
require 'soap/wsdlDriver'
require 'xsd/mapping'

SERVICE_ROOT = 'http://localhost:9999/sipxconfig/services'
USER_SERVICE_WSDL = SERVICE_ROOT + '/UserService?wsdl'

class UserServiceTestApi < Test::Unit::TestCase

    def setup
		@userService = SOAP::WSDLDriverFactory.new(USER_SERVICE_WSDL).create_rpc_driver
		@userService.wiredump_dev = STDOUT if $DEBUG
    end

	def test_createUser
		@userService.nop(:hello =>'hello')
    end
end