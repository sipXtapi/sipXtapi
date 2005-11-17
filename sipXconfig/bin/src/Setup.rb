#!/usr/bin/ruby

require 'stringio'
require 'fileutils'

# Setup sipXconfig by integrating settings into local system
class Setup

  attr_reader :options
  
  # setup tftp to work with sipXconfig
  def setup()
    setup_tftp()
  end

  def read()
    read_tftp()
    save()
  end

  def initialize() 
    @options = {}
  end

  def load(properties_file)
    @options = Properties.new().load(properties_file)
  end

  # Install hooks into systems TFTP server so sipXconfig
  # and leverage service.
  def setup_tftp()

    # redhat/fedora using xinet.d
    installer = FileInstaller.new()
    installer.backup_dir = require_property('sysdir.tmp')
    installer.open(tftpConfig)
    setup_xinet(installer.infile, installer.outfile)
    installer.install()

  end

  def tftpConfig()
    return property('sysdir.tftpConfig', '/etc/xinet.d/tftp')
  end

  def property(key, defaultValue)
    value = @options[key]
    return value ? value : defaultValue
  end

  def require_property(key)
    value = @options[key]
    if (!value)
      raise TypeError.new("missing, required property : `#{key}'")
    end
    return value
  end

  TFTP_ROOT_REGEXP = /^\s*server_args\s*=.*-s\s*([^\s]+)/

  # enable TFTP and make it run as proper user
  def setup_xinet(infile, outfile)
    infile.each_line do |line|
      outfile.puts case line
        when /disable\s*=\s*yes/x then
          line.sub("yes", "no")
        when TFTP_ROOT_REGEXP then
          @options['sysdir.tftpRoot'] = $1
          line
        else line
      end
    end

    # sipxpbx user needs to write to tftp root
    user = require_property('sipxpbx.user')
    # nice to find way to unittest this
    FileUtils.chown(user, nil, @options['sysdir.tftpRoot']) if defined? $UNITTEST

  end

  def read_xinet(infile)
    infile.each_line do |line|
      case line
        when TFTP_ROOT_REGEXP then
          @options['sysdir.tftpRoot'] = $1
      end
    end    
  end

end

# When making edits to file, make backup of original file 
# and manage io streams for utils to r/w to original file
# edit will not persist until all edits complete.
#
# NOTE: This could probably be written better by allowing
# caller to do this
#
#   FileInstaller.install(in, [backupdir]) { |infile, outfile|
#       custom code here, closes streams on closure exit
#   }
#  but i do not know how to do that yet.
#
class FileInstaller

  attr_reader :infile
  attr_reader :outfile
  attr_writer :backup_dir

  def initialize()
  end
  
  def open(source)
    @source = source
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

# Reading/writing of java properties files into a ruby hash
#
# NOTE: There should be a away to 'mix-in' hash into Properties
# class but I don't know how.  (e.g. self[key] = param) so I'm returning
# the hash
#
class Properties 

  KEY_REGSRC = '([^=:\\\\]*(?:\\\\.[^=:\\\\]*)*)'
  DEF_REGSRC = '\\s*' + KEY_REGSRC + '\\s*[=:]\\s*(.*)'
  COMMENT_REGEXP = Regexp.new('^(?:#.*|)$')
  LINE_REGEXP = Regexp.new("^#{DEF_REGSRC}$")

  attr_reader :props
  def load(properties_file)
    @props = Hash.new
    @properties_file = properties_file
    File.open(@properties_file).readlines.each_with_index do |line, lineno|
      line.sub!(/\r?\n\z/, '')
      case line
      when COMMENT_REGEXP
        next
      when LINE_REGEXP
        key, value = $1.strip, $2.strip
        @props[key] = value
      else
        raise TypeError.new("property format error at line #{lineno + 1}: `#{line}'")
      end
    end
    return @props
  end

  def save(properties_file)
    File.open(properties_file, "w+") {|file|
      @props.each do |key, value|
        file.puts key + " = " + value
      end
   }
  end
end
