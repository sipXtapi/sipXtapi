require 'test/unit'
require 'ConfigServiceDriver.rb'

SERVICE_ROOT = 'http://localhost:9999/sipxconfig/services'
PARK_ORBIT_SERVICE_WSDL = SERVICE_ROOT + '/ParkOrbitService'
TEST_SERVICE_WSDL = SERVICE_ROOT + '/TestService'

DUMMY_USER = "dummy user"
DUMMY_PASSWORD = "dummy password"

# Extend the ParkOrbit by overriding the equality operator
class ParkOrbit
  def ==(other)
    (self.name == other.name) &&
    (self.extension == other.extension) &&
    (self.description == other.description) &&
    (self.enabled == other.enabled) &&
    (self.music == other.music)
  end    
end

class ParkOrbitServiceTestApi < Test::Unit::TestCase

    def setup
		@parkOrbitService = ParkOrbitService.new(PARK_ORBIT_SERVICE_WSDL)
		@parkOrbitService.wiredump_dev = STDOUT if $DEBUG
        @parkOrbitService.options["protocol.http.basic_auth"] << [PARK_ORBIT_SERVICE_WSDL, DUMMY_USER, DUMMY_PASSWORD]

		@testService = TestService.new(TEST_SERVICE_WSDL)
        @testService.options["protocol.http.basic_auth"] << [TEST_SERVICE_WSDL, DUMMY_USER, DUMMY_PASSWORD]
		
		reset = ResetServices.new();
		reset.parkOrbit = true;
		@testService.resetServices(reset)
    end
    
	def test_createAndGetParkOrbits
	    # Create two ParkOrbits
	    # ParkOrbit properties are name, extension, description, enabled, music
	    expected1 = ParkOrbit.new('orbit1', 'ext1', 'test orbit1', true, 'love supreme');
		@parkOrbitService.addParkOrbit(AddParkOrbit.new(expected1))
	    expected2 = ParkOrbit.new('orbit2', 'ext2', 'test orbit2', true, 'walk like a tunisian');
		@parkOrbitService.addParkOrbit(AddParkOrbit.new(expected2))

        # Get ParkOrbits and verify that they are right
	    getParkOrbitsResponse = @parkOrbitService.getParkOrbits()
	    parkOrbits = getParkOrbitsResponse.parkOrbits
	    assert_equal(2, parkOrbits.length)
	    assert_equal(expected1, parkOrbits[0])	    
	    assert_equal(expected2, parkOrbits[1])	    
    end
end
