require 'test/unit'
require 'ConfigServiceDriver.rb'

SERVICE_ROOT = 'http://localhost:9999/sipxconfig/services'
USER_SERVICE_WSDL = SERVICE_ROOT + '/UserService'
TEST_SERVICE_WSDL = SERVICE_ROOT + '/TestService'

class UserServiceTestApi < Test::Unit::TestCase

    def setup
		@userService = UserService.new(USER_SERVICE_WSDL)
		@userService.wiredump_dev = STDOUT if $DEBUG

		@testService = TestService.new(TEST_SERVICE_WSDL)
		@testService.wiredump_dev = STDOUT if $DEBUG
		@testService.resetUserService()
    end

	def test_createUser
	    @user = User.new('lipton', '1234')
		@userService.addUser(AddUser.new(@user))
    end
end