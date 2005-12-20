require 'test/unit'
require 'flexmock'
require 'tempfile'
require 'fileutils'
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
    files = [ "file.backup"]
    system.should_receive(:configuration_files).and_return(files)
    system.should_receive(:unresolved_backup_files).with(files).and_return(files)
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
    system.should_receive(:delete_file).with("file.backup")
    
    setup.finish_upgrade
  end
end


class SystemServiceTest < Test::Unit::TestCase
public
  def setup
    @system = SystemService.new    
  end
  def test_create_patch
    original = <<'__ORIGINAL__'
ABC
__ORIGINAL__

    modified = <<'__MODIFIED__'
ABC
DEF
__MODIFIED__

    patch = @system.create_patch(temp_file(original), temp_file(original))
    assert_equal('', patch)

    patch = @system.create_patch(temp_file(original), temp_file(modified))
    assert_not_equal('', patch)
  end
  
  def test_apply_patch

    patch = <<'__PATCH__'
--- foo.original
+++ foo.modified
@@ -1,2 +1,3 @@
 FOO
 BAR
+GOO
  
__PATCH__

    master = <<'__MASTER__'
FOO
BAR
__MASTER__

    master_file = temp_file(master)
    assert @system.test_patch(master_file, patch)
    
    patched_master = <<'__PATCHED_MASTER__'
FOO
BAR
GOO
__PATCHED_MASTER__

    @system.apply_patch(master_file, patch)
    assert_equal file_contents(master_file), patched_master
  end
  
  def file_contents(file)
    return File.open(file).read
  end
  
  def temp_file(contents) 
    temp = Tempfile.new("sipxsetup-test")
    temp.write contents
    temp.close
    return temp.path
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
  def test_unresolved_backup_files
  end
end
