require 'test/unit'
require 'ConfigServiceDriver.rb'

SERVICE_ROOT = 'http://localhost:9999/sipxconfig/services'
PHONE_SERVICE_WSDL = SERVICE_ROOT + '/PhoneService'
TEST_SERVICE_WSDL = SERVICE_ROOT + '/TestService'

class PhoneServiceTestApi < Test::Unit::TestCase

    def setup
		@phoneService = PhoneService.new(PHONE_SERVICE_WSDL)
		@phoneService.wiredump_dev = STDOUT if $DEBUG

		@testService = TestService.new(TEST_SERVICE_WSDL)
		@testService.wiredump_dev = STDOUT if $DEBUG
		reset = ResetService.new();
		reset.phone = true;
		@testService.resetService(reset)
    end

	def test_createPhone	    
	    expected = Phone.new('000000000000', 'unmanagedPhone')
	    addPhone = AddPhone.new(expected)
	    addPhone.group = [ 'group1' ]
		@phoneService.addPhone(addPhone)

	    findPhone = FindPhone.new(PhoneSearch.new())	  
	    findPhone.search.bySerialNumber = expected.serialNumber
	    phones = @phoneService.findPhone(findPhone).phones
	    
	    assert_equal(1, phones.length)
	    assert_equal(expected.serialNumber, phones[0].serialNumber)	    
    end
end