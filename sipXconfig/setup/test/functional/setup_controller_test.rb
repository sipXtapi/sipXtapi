require File.dirname(__FILE__) + '/../test_helper'
require 'setup_controller'

# Re-raise errors caught by the controller.
class SetupController; def rescue_action(e) raise e end; end

class SetupControllerTest < Test::Unit::TestCase
  def setup
    @controller = SetupController.new
    @request    = ActionController::TestRequest.new
    @response   = ActionController::TestResponse.new
  end

  # Replace this with your real tests.
  def test_truth
    assert true
  end
end
