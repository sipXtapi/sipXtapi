require 'test/unit'
require 'ConfigServiceDriver.rb'
require 'ApiTestCase.rb'

# Extend the CallGroup by overriding the equality operator so we can check equality by value
class CallGroup
  def ==(other)
   (self.name == other.name) &&
    (self.extension == other.extension) &&
     (self.description == other.description) &&
      (self.enabled == other.enabled) &&
       (self.rings == other.rings)
  end    
end

# Extend the UserRing by overriding the equality operator so we can check equality by value
class UserRing
  def ==(other)
   (self.expiration == other.expiration) &&
    (self.type == other.type) &&
     (self.position == other.position) &&
      (self.userName == other.userName)
  end
end

class CallGroupServiceTestApi < ApiTestCase  
  def setup
    super
    
    # create the services that we will use
    @call_group_service = service(CallGroupService)
    @test_service = service(TestService)
    @user_service = service(UserService)

    # reset those services to start with a clean slate    
    reset = ResetServices.new();
    reset.callGroup = true;
    reset.user = true;
    @test_service.resetServices(reset)
  end
  
  def test_createAndGetCallGroups
    # Create a User so that we can make a UserRing for the CallGroup.
    # When we call the service to add the CallGroup and thus the associated UserRing,
    # the UserRing's userName must match a user in the DB.
    user = User.new('lipton', '1234')
    addUser = AddUser.new(user)
    @user_service.addUser(addUser)

    # Create a UserRing
    # UserRing properties are expiration, type, position, userName
    inRing = UserRing.new(19, 'immediate', 99, 'lipton')

    # Create a CallGroup
    # CallGroup properties are name, extension, description, enabled, rings
    inCallGroup = CallGroup.new('callGroup1', 'ext1', 'test callGroup1', true, [ inRing ]);
    @call_group_service.addCallGroup(AddCallGroup.new(inCallGroup))
    
    # Call getCallGroups and verify the result
    getCallGroupsResponse = @call_group_service.getCallGroups()
    callGroups = getCallGroupsResponse.callGroups
    assert_equal(1, callGroups.length)
    assert_equal(inCallGroup, callGroups[0])	    
  end
end
