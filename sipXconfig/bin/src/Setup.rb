#!/usr/bin/ruby

require 'stringio'
require 'FileUtils'

# Setup sipXconfig by integrating settings into local system
class Setup

  attr_reader :tftpRoot
  attr_writer :installer
  
  # setup tftp to work with sipXconfig
  def setup()
    setupTftp()
  end
    
  # Install hooks into systems TFTP server so sipXconfig
  # and leverage service.
  def setupTftp()

    # redhat/fedora using xinet
    @installer.open('/etc/xinet.d/tftp')
    setup_xinet(@installer.infile, @installer.outfile)
    @installer.install()
    
  end
    
  # enable TFTP and make it run as proper user
  def setup_xinet(infile, outfile)
    enable = /disable\s*=\s*yes/x
    sipxuser = /user\s*=\s*root/x
    while line = infile.gets() do
      outfile.puts line.
	  gsub(enable, "disable\t\t\t= no").
	  gsub(sipxuser, "user\t\t\t= sipxchange")

      if /^\s*server_args\s*.*/.match(line)
	items = line.split()
	@tftpRoot = items[items.length - 1]
      end
    end
  end

end

# When making edits to file, make backup of original file 
# and manage io streams for utils to r/w to original file
# edit will not persist until all edits complete.
class FileInstaller

  attr_reader :infile, :outfile
  attr_writer :system_root, :backup_dir
  
  def initialize() 
    @system_root = ''
  end

  def open(source)
    @source = @system_root + source
    @infile = File.open(@source)
    @outfile = StringIO.new()
  end

  def install()
    @infile.close()
    backup = nil
    if (@backup_dir)
      FileUtils.mkdir_p(@backup_dir)
      backup = File.join(@backup_dir, File.basename(@source))
      FileUtils.copy_file(@source, backup)
    end
    File.open(@source, "w+"){ |file|
      file.write outfile.string
    }
    infile = nil
    outfile = nil
    return backup
  end
end

