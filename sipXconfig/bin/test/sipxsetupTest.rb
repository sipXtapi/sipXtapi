require 'test/unit'
require 'rubygems'
require 'flexmock'
require 'tempfile'
require 'fileutils'
load 'sipxsetup'

class SetupTest < Test::Unit::TestCase
public
  def setup
    @system = FlexMock.new("sipxsetup")
    @setup = Setup.new(@system)    	
  end
  def
    @system.verify
  end
  def test_show_modified
    @system.should_receive(:packages).and_return(["p1", "p2"])
    @system.should_receive(:modified_files).with("p1").and_return(["f1"])
    @system.should_receive(:console).with("f1")
    @system.should_receive(:modified_files).with("p2").and_return([])
    
    @setup.show_modified
  end
  def test_prepare_for_upgrade
    @system.should_receive(:perform_backup)
    @setup.prepare_for_upgrade
  end
  def test_finish_upgrade
    files = [ "file.master"]
    @system.should_receive(:packages).and_return("foo")
    @system.should_receive(:configuration_files).with("foo").and_return(files)
    @system.should_receive(:backup_filename).with("file.master").and_return("file.backup")
    @system.should_receive(:file_exists).with("file.backup").and_return(true)
    @system.should_receive(:create_patch).with("file.master", "file.backup").and_return("patch")
    @system.should_receive(:test_patch).with("file.master", "patch").and_return(true)
    @system.should_receive(:apply_patch).with("file.master", "patch")
    @system.should_receive(:backup_filename).with("file.master").and_return("file.backup")
    @system.should_receive(:delete_file).with("file.backup")
    
    assert @setup.finish_upgrade
  end
  def test_unresolved_backup_files
    files = [ "file1", "file2"]
    @system.should_receive(:packages).and_return("foo")
    @system.should_receive(:configuration_files).with("foo").and_return(files)
    @system.should_receive(:backup_filename).with("file1").and_return("backup1")
    @system.should_receive(:file_exists).with("backup1").and_return(true)
    @system.should_receive(:backup_filename).with("file2").and_return("backup2")
    @system.should_receive(:file_exists).with("backup2").and_return(false)
    assert_equal(["file1"], @setup.unresolved_backup_files)
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
  def test_backup_filename
    assert_equal("foo.rpmnew", @system.backup_filename("foo"))
  end
end
