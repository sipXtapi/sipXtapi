require 'test/unit'
require 'flexmock'
load 'sipxsetup'

class SetupTest < Test::Unit::TestCase
public
  def test_show_modified
    system = FlexMock.new("sipxsetup")
    setup = Setup.new(system)  
    system.should_receive(:packages).and_return {
      [ "sipxportlib", "sipxtacklib" ]      
    }
    system.should_receive(:modified_files).with("sipxportlib").and_return {
      [ "sipxport.properties" ]      
    }
    system.should_receive(:puts).with("sipxport.properties")
    system.should_receive(:modified_files).with("sipxtacklib").and_return {
      [ "sipxtack.properties" ]      
    }
    system.should_receive(:puts).with("sipxtack.properties")
    
    setup.show_modified
  end
  def test_prepare_for_upgrade
    system = FlexMock.new("sipxsetup")
    setup = Setup.new(system)  
    system.should_receive(:perform_backup)

    setup.prepare_for_upgrade
  end
  def test_finish_upgrade
    system = FlexMock.new("sipxsetup")
    setup = Setup.new(system)  
    system.should_receive(:unresolved_backup_files).and_return {
      [ "file.backup"]
    }    
    system.should_receive(:backup_master_file).with("file.backup").and_return {
      "file.master"
    }    
    system.should_receive(:create_patch).with("file.master", "file.backup").and_return {
      "patch"
    }
    system.should_receive(:test_patch).with("file.master", "patch").and_return {
      true
    }
    system.should_receive(:apply_patch).with("file.master", "patch")
    
    setup.finish_upgrade
  end
end

class RedhatSystemTest < Test::Unit::TestCase
public
  def setup
    @system = RedhatSystem.new
  end
  def test_unresolved_master
    assert_equal("foo", @system.backup_master_file("foo.rpmsave"))
  end
end
