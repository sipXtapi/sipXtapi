require 'test/unit'
require 'Tempfile'
require 'stringio'
load 'Setup.rb'

SYSTEM_ROOT = 'test/system_root'
BACKUP_DIR = Dir::tmpdir + '/test-setup-backup'

class FileInstallerTest < Test::Unit::TestCase
public
  def test_install
    source = Dir::tmpdir + '/x'
    File.open(source, "w+") { |file|
        file.write "original"
    }
    fixer = FileInstaller.new();
    fixer.backup_dir = BACKUP_DIR
    fixer.open(source)
    fixer.outfile.write "fixed"
    backup = fixer.install()
    
    assert_equal("fixed", File.open(source).read)
    assert_equal("original", File.open(backup).read)
  end
end

class SetupTest < Test::Unit::TestCase
public
  def test_setup
    installer = FileInstaller.new()
    installer.system_root = SYSTEM_ROOT
    installer.backup_dir = BACKUP_DIR

    tftp = Setup.new()
    tftp.installer = installer
    tftp.setupTftp()
    backup = BACKUP_DIR + '/tftp'
    installed = SYSTEM_ROOT + '/etc/xinet.d/tftp'
    assert(File.exists?(backup))
    assert(FileUtils.compare_file(backup, installed))
  end

  def test_setup_xinit
    tftp = Setup.new()
    out = StringIO.new();
    repair = tftp.setup_xinet(File.open('test/tftp.orig'), out)
    out.seek(0)
    assert_file_equal(File.open('test/tftp.expected'), out)
    assert_equal('/tftpboot', tftp.tftpRoot)
  end

  def assert_file_equal(expected, actual) 
    while line = expected.gets
      assert_equal(line, actual.gets)
    end
  end

end
