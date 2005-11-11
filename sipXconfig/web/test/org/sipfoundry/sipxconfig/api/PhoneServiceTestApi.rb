require 'test/unit'
require 'ConfigServiceDriver.rb'

SERVICE_ROOT = 'http://localhost:9999/sipxconfig/services'
PHONE_SERVICE_WSDL = SERVICE_ROOT + '/PhoneService'
USER_SERVICE_WSDL = SERVICE_ROOT + '/UserService'
TEST_SERVICE_WSDL = SERVICE_ROOT + '/TestService'

DUMMY_USER = "dummy user"
DUMMY_PASSWORD = "dummy password"

class PhoneServiceTestApi < Test::Unit::TestCase

    def setup
		@phoneService = PhoneService.new(PHONE_SERVICE_WSDL)
		@phoneService.wiredump_dev = STDOUT if $DEBUG
        @phoneService.options["protocol.http.basic_auth"] << [PHONE_SERVICE_WSDL, DUMMY_USER, DUMMY_PASSWORD]

		@userService = UserService.new(USER_SERVICE_WSDL)
        @userService.options["protocol.http.basic_auth"] << [USER_SERVICE_WSDL, DUMMY_USER, DUMMY_PASSWORD]
		@testService = TestService.new(TEST_SERVICE_WSDL)
        @testService.options["protocol.http.basic_auth"] << [TEST_SERVICE_WSDL, DUMMY_USER, DUMMY_PASSWORD]
		
		reset = ResetService.new();
		reset.phone = true;
		reset.user = true;
		@testService.resetService(reset)
    end

	def test_createPhone	    
	    expected = Phone.new('000000000000', 'unmanagedPhone')
	    expected.groups = [ 'group1' ]
		@phoneService.addPhone(AddPhone.new(expected))

	    findPhone = FindPhone.new(PhoneSearch.new())	  
	    findPhone.search.bySerialNumber = expected.serialNumber
	    phones = @phoneService.findPhone(findPhone).phones
	    
	    assert_equal(1, phones.length)
	    assert_equal(expected.serialNumber, phones[0].serialNumber)	    
    end
    
    def seedPhone
	    @seed = Phone.new('000000000000', 'unmanagedPhone')
	    addPhone = AddPhone.new(@seed)
		@phoneService.addPhone(addPhone)
    end
    
    def test_addLine
	    seedPhone()
    	addUser1 = AddUser.new(User.new('user1'), '1234')
		@userService.addUser(addUser1)

		search = PhoneSearch.new(@seed.serialNumber)
		edit = AdminPhone.new(search);
		edit.addLine = Line.new('user1')
		@phoneService.adminPhone(edit)
		
		phone = @phoneService.findPhone(FindPhone.new(search)).phones[0]
		assert_equal(phone.lines[0].userName, 'user1')
    end
    
    def test_deletePhone
	    seedPhone()

	    # ensure phone was created
	    findPhone = FindPhone.new(PhoneSearch.new())	  
	    findPhone.search.bySerialNumber = @seed.serialNumber
	    assert_equal(1, @phoneService.findPhone(findPhone).phones.length)

	    deletePhone = AdminPhone.new(PhoneSearch.new())
	    deletePhone.search.bySerialNumber = @seed.serialNumber
	    deletePhone.deletePhone = true
	    @phoneService.adminPhone(deletePhone)
	    
	    # ensure phone was deleted
	    assert_equal(0, @phoneService.findPhone(findPhone).phones.length)
    end
    
    def test_phoneProfileAndRestart
    end
end