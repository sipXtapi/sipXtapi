require 'test/unit'
require 'tempfile'
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

class PropertiesTest < Test::Unit::TestCase

  def test_load
    setup = Properties.new()
    props = setup.load('test/sample.properties')
    assert_equal('me', props['found'])
    assert_equal('first = last', props['param-with-equals'])
  end

  def test_save
    setup = Properties.new()
    props = setup.load('test/sample.properties')
    props['me'] = 'updated value'
    props['new-key'] = 'new value'

    actual_file = Dir::tmpdir + '/save.properties'
    setup.save(actual_file)
    assert(FileUtils.compare_file('test/expected-save.properties', actual_file))
  end

end

class SetupTest < Test::Unit::TestCase
public

  def test_setup
    installer = FileInstaller.new()
    installer.backup_dir = BACKUP_DIR

    setup = Setup.new()
    expected = 'test/tftp.simple'
    setup.options['sysdir.tftpConfig'] = expected
    setup.options['sysdir.tmp'] = BACKUP_DIR
    setup.setup_tftp()
    backup_file = BACKUP_DIR + '/tftp.simple'
    assert(File.exists?(backup_file))
    assert(FileUtils.compare_file(expected, backup_file))
  end

  def test_setup_xinet
    setup = Setup.new()
    setup.options['sipxpbx.user'] = 'homer'
    out = StringIO.new();
    repair = setup.setup_xinet(File.open('test/tftp.orig'), out)
    out.seek(0)
    assert_file_equal(File.open('test/tftp.expected'), out)
    assert_equal('/tftpboot', setup.options['sysdir.tftpRoot'])
  end

  def assert_file_equal(expected, actual) 
    while line = expected.gets
      assert_equal(line, actual.gets)
    end
  end

end
