require 'test/unit'
require 'UserServiceDriver.rb'

SERVICE_ROOT = 'http://localhost:9999/sipxconfig/services'
USER_SERVICE_WSDL = SERVICE_ROOT + '/UserService?wsdl'

class UserServiceTestApi < Test::Unit::TestCase

    def setup
		@userService = UserService.new(USER_SERVICE_WSDL)
		@userService.wiredump_dev = STDOUT if $DEBUG
    end

	def test_createUser
		addUser = AddUser.new('joeyxz', '1234');
		@userService.addUser(addUser)
    end
end