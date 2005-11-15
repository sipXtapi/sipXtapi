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
		
		reset = ResetServices.new();
		reset.phone = true;
		reset.user = true;
		@testService.resetServices(reset)
    end
    
	def test_createPhone	    
	    expected = Phone.new('000000000000', 'unmanagedPhone')
	    expected.groups = [ 'group1' ]
		@phoneService.addPhone(AddPhone.new(expected))

	    findPhone = FindPhone.new(PhoneSearch.new(expected.serialNumber))	  
	    phones = @phoneService.findPhone(findPhone).phones
	    
	    assert_equal(1, phones.length)
	    assert_equal(expected.serialNumber, phones[0].serialNumber)	    
	    assert_equal(1, phones[0].groups.length)
	    assert_equal('group1', phones[0].groups[0])
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

    def test_addRemoveGroup	
        seedPhone()
        
	    search = PhoneSearch.new(@seed.serialNumber)

        #add group
        addGroup = AdminPhone.new(search)
        addGroup.addGroup = 'group1'
        @phoneService.adminPhone(addGroup)
	    phones = @phoneService.findPhone(FindPhone.new(search)).phones;
	    assert_equal('group1', phones[0].groups[0])
	    
	    #remove group
        removeGroup = AdminPhone.new(search)
        removeGroup.removeGroup = 'group1'
        @phoneService.adminPhone(removeGroup)
	    phones = @phoneService.findPhone(FindPhone.new(search)).phones;
	    assert_nil(phones[0].groups)	    
    end

    def seedPhone
	    @seed = Phone.new('000000000000', 'unmanagedPhone')
	    addPhone = AddPhone.new(@seed)
		@phoneService.addPhone(addPhone)
    end
    
    def test_addRemoveLine
	    seedPhone()
    	addUser1 = AddUser.new(User.new('user1'), '1234')
		@userService.addUser(addUser1)

        # add line
		search = PhoneSearch.new(@seed.serialNumber)
		addLine = AdminPhone.new(search);
		addLine.addLine = Line.new('user1')
		@phoneService.adminPhone(addLine)
		
		phone = @phoneService.findPhone(FindPhone.new(search)).phones[0]
		assert_equal(phone.lines[0].userName, 'user1')

		#remove line		
		removeLine = AdminPhone.new(search);
		removeLine.removeLine = 'user1'
		@phoneService.adminPhone(removeLine)
		
		phone = @phoneService.findPhone(FindPhone.new(search)).phones[0]
		assert_nil(phone.lines)
    end
    
    def test_phoneEdit
        seedPhone()

		search = PhoneSearch.new(@seed.serialNumber)
		edit = AdminPhone.new(search);
		edit.edit = [ Property.new('description', 'hello') ]
		@phoneService.adminPhone(edit)
		
		phone = @phoneService.findPhone(FindPhone.new(search)).phones[0]
		assert_equal(phone.description, 'hello')       
    end
        
    def test_phoneProfileAndRestart
    	addUser1 = AddUser.new(User.new('user1'), '1234')
		@userService.addUser(addUser1)

	    @seed = Phone.new('000000000000', 'polycom300')
	    addPhone = AddPhone.new(@seed)
	    @seed.lines = [ Line.new('user1') ]
		@phoneService.addPhone(addPhone)
		
		generateProfiles = AdminPhone.new()
		generateProfiles.generateProfiles = true
		@phoneService.adminPhone(generateProfiles)
		# this just excersizes code. does not verify profiles were generated
		
		restart = AdminPhone.new()
		restart.restart = true
		@phoneService.adminPhone(restart)
		# this just excersizes code. does not verify restart message was sent	
    end
    
    def test_phoneSearch
        seedPhone()
        all = @phoneService.findPhone(FindPhone.new()).phones
        assert_equal(1, all.length)
        assert_equal(@seed.serialNumber, all[0].serialNumber)        
    end
end