require 'test/unit'
require 'ConfigServiceDriver.rb'

SERVICE_ROOT = 'http://localhost:9999/sipxconfig/services'
USER_SERVICE_WSDL = SERVICE_ROOT + '/UserService'
TEST_SERVICE_WSDL = SERVICE_ROOT + '/TestService'

DUMMY_USER = "dummy user"
DUMMY_PASSWORD = "dummy password"

class UserServiceTestApi < Test::Unit::TestCase

    def setup
		@userService = UserService.new(USER_SERVICE_WSDL)
		@userService.wiredump_dev = STDOUT if $DEBUG
        @userService.options["protocol.http.basic_auth"] << [USER_SERVICE_WSDL, DUMMY_USER, DUMMY_PASSWORD]

		@testService = TestService.new(TEST_SERVICE_WSDL)
		@testService.wiredump_dev = STDOUT if $DEBUG
        @testService.options["protocol.http.basic_auth"] << [TEST_SERVICE_WSDL, DUMMY_USER, DUMMY_PASSWORD]
		reset = ResetService.new();
		reset.user = true;
		@testService.resetService(reset)
    end

	def test_createUser	    
	    expected = User.new('lipton', '1234')
	    addUser = AddUser.new(expected)
	    expected.groups = [ 'group1' ]
		@userService.addUser(addUser)

	    findUser = FindUser.new(UserSearch.new())
	    findUser.search.byUserName = expected.userName
	    users = @userService.findUser(findUser).users
	    
	    assert_equal(1, users.length)
	    assert_equal(expected.userName, users[0].userName)		
    end
    
	def test_findUser
	    user1 = User.new('joe')
		@userService.addUser(AddUser.new(user1, '1234'))
	    user2 = User.new('joe-boy')
		@userService.addUser(AddUser.new(user2, '1234'))
	    user3 = User.new('mary-joe')
		@userService.addUser(AddUser.new(user3, '1234'))

	    findUser = FindUser.new(UserSearch.new())
	    findUser.search.byUserName = 'joe'
	    users = @userService.findUser(findUser).users
	    assert_equal(1, users.length)
    end
    
	def test_deleteUser
		@userService.addUser(AddUser.new(User.new('john'), '1234'))

		search = UserSearch.new();
	    search.byUserName = 'john'
	    users = @userService.findUser(FindUser.new(search)).users
	    assert_equal(1, users.length)
	    
	    deleteUser = AdminUser.new(search);
	    deleteUser.deleteUser = true
	    @userService.adminUser(deleteUser)
	    
	    users = @userService.findUser(FindUser.new(search)).users
	    assert_equal(0, users.length)
    end
    
    def test_addGroup             
    	addUser1 = AddUser.new(User.new('user1'), '1234')
		@userService.addUser(addUser1)    

	    search = UserSearch.new(addUser1.user.userName)
		users = @userService.findUser(FindUser.new(search)).users
	    assert_nil(users[0].groups)
	    
	    addGroup = AdminUser.new(search);
	    addGroup.addGroup = 'yaya'
	    @userService.adminUser(addGroup)

		users = @userService.findUser(FindUser.new(search)).users
	    assert_equal(1, users[0].groups.length)
	    assert_equal('yaya', users[0].groups[0])
    end
    
    def test_removeGroup	                      
    	addUser1 = AddUser.new(User.new('user1'), '1234')
    	addUser1.user.groups = [ 'group1', 'group2' ]
		@userService.addUser(addUser1)    
	    
	    search = UserSearch.new(addUser1.user.userName)  
	    removeGroup = AdminUser.new(search);
	    removeGroup.removeGroup = 'group2'
	    @userService.adminUser(removeGroup)

		users = @userService.findUser(FindUser.new(search)).users
	    assert_equal(1, users[0].groups.length)
	    assert_equal('group1', users[0].groups[0])
    end
    
	def test_editUser
    	addUser1 = AddUser.new(User.new('user1'), '1234')
	    addUser1.user.firstName = 'Ali'
	    addUser1.user.lastName = 'Baba'
	    addUser1.user.groups = [ 'group1', 'group2' ]
		@userService.addUser(addUser1)

	    addUser2 = AddUser.new(User.new('user2'), '1234')
	    addUser2.user.firstName = 'Holy'
	    addUser2.user.lastName = 'Mackerel'
	    addUser2.user.groups = [ 'group1', 'group3' ]
		@userService.addUser(addUser2)

	    addUser3 = AddUser.new(User.new('user3'), '1234')
	    addUser3.user.firstName = 'Sim'
	    addUser3.user.lastName = 'Salabim'
	    addUser3.user.groups = [ 'group2', 'group3' ]
		@userService.addUser(addUser3)
		
	    search = UserSearch.new()
	    search.byGroup = 'group3'
		editUser = AdminUser.new(search)
		editUser.edit = [ 
		    Property.new('firstName', 'Secret'), 
		    Property.new('lastName', 'Agent') 
		]
	    users = @userService.adminUser(editUser)
	    
	    users = @userService.findUser(FindUser.new(nil)).users
	    users.sort do |a, b|
	      a.userName <=> b.userName
	    end
	    assert_equal('Ali', users[0].firstName)
	    assert_equal('Baba', users[0].lastName)
	    assert_equal('Secret', users[1].firstName)
	    assert_equal('Agent', users[1].lastName)
	    assert_equal('Secret', users[2].firstName)
	    assert_equal('Agent', users[2].lastName)
    end
end