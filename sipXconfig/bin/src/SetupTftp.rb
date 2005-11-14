#!/usr/bin/ruby

# Install hooks into systems TFTP server so sipXconfig
# and leverage service.
class SetupTftp

  attr_reader :root

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
        @root = items[items.length - 1]
      end
    end
  end

end
