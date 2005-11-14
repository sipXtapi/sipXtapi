require 'test/unit'
require 'tempfile'
require 'SetupTftp.rb'

class SetupTftpTest < Test::Unit::TestCase

public

  def test_setup
    tftp = SetupTftp.new()
    out = Tempfile.new('x');
    repair = tftp.setup_xinet(File.open('test/tftp.orig'), out)
    out.close();
    assert_file_equal(File.open('test/tftp.expected'), out.open())
    assert_equal('/tftpboot', tftp.root)
  end

  def assert_file_equal(expected, actual) 
    while line = expected.gets
      assert_equal (line, actual.gets)
    end
  end

end
