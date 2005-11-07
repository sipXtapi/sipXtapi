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
	    expected = User.new('lipton', '1234')
	    addUser = AddUser.new(expected)
	    addUser.group = [ 'group1' ]
		@userService.addUser(addUser)

	    findUser = FindUser.new()
	    findUser.byName = expected.userName
	    users = @userService.findUser(findUser).users
	    
	    assert_equal(1, users.length)
	    assert_equal(expected.userName, users[0].userName)
		
    end
    
	def test_findUser
	    user1 = User.new('john', '1234')
		@userService.addUser(AddUser.new(user1))
	    user2 = User.new('john-boy', '1234')
		@userService.addUser(AddUser.new(user2))
	    user2 = User.new('mary', '1234')
		@userService.addUser(AddUser.new(user2))

	    findUser = FindUser.new('john')
	    users = @userService.findUser(findUser).users

	    assert_equal(2, users.length)	    
    end
    
	def test_deleteUser
	    user1 = User.new('john', '1234')
		@userService.addUser(AddUser.new(user1))

	    findUser = FindUser.new('john')
	    users = @userService.findUser(findUser).users
	    assert_equal(1, users.length)
	    
	    @userService.deleteUser(DeleteUser.new('john'))
	    
	    users = @userService.findUser(findUser).users
	    assert_equal(0, users.length)
    end

	def test_editUser
	    user1 = User.new('john', '1234')
	    user1.firstName = 'Johny'
	    user1.lastName = 'Quest'
		@userService.addUser(AddUser.new(user1))
		
		editUser = EditUser.new('john')
		editUser.properties = [ Property.new('firstName', 'Secret'), 
		    Property.new('lastName', 'Agent') ]

	    users = @userService.editUser(editUser)	    
	    
	    findUser = FindUser.new('john')
	    user = @userService.findUser(findUser).users[0]
	    assert_equal('Secret', user.firstName)
	    assert_equal('Agent', user.lastName)
    end
end