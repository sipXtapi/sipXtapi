#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require File.join(File.dirname(__FILE__), '..', 'test_helper')
require File.join(File.dirname(__FILE__), '..', '..', 'database_url')


class DatabaseUrlTest < Test::Unit::TestCase
  
  def setup
    @u1 = DatabaseUrl.new('a', 'b', 'c', 'd', 'e')
    @u2 = DatabaseUrl.new('a', 'b', 'c', 'd', 'e')
    @u3 = DatabaseUrl.new('z', 'b', 'c', 'd', 'e')
    @u4 = DatabaseUrl.new(nil, 'b', 'c', 'd', 'e')    
    @u5 = DatabaseUrl.new('a', 'b', 'c', 'd', 'z')    
  end
  
  def test_create_url
    url = DatabaseUrl.new('database', 'port', 'adapter', 'host', 'username')
    assert_equal('database', url.database)
    assert_equal('port', url.port)
    assert_equal('adapter', url.adapter)
    assert_equal('host', url.host)
    assert_equal('username', url.username)
    
    h = url.to_hash
    assert_equal(5, h.size());
    assert_equal('database', h[:database])
    assert_equal('port', h[:port])
    assert_equal('adapter', h[:adapter])
    assert_equal('host', h[:host])
    assert_equal('username', h[:username])
  end
  
  def test_equality
    assert_equal(@u1, @u2)
    assert(@u1.eql?(@u2))
    assert_not_equal(@u1, @u3)
    assert_not_equal(@u1, @u4)
    assert_not_equal(@u1, @u5)
    assert_not_equal(@u2, @u5)
    assert_not_equal(@u4, @u3)
  end
  
  def test_hash
    assert_equal('a'.hash + 'b'.hash + 'c'.hash + 'd'.hash + 'e'.hash,
                 @u1.hash)
    assert_equal(@u1.hash, @u2.hash)
    assert_not_equal(@u1.hash, @u3.hash)
    assert_not_equal(@u1.hash, @u4.hash)
    assert_not_equal(@u1.hash, @u5.hash)
    assert_not_equal(@u2.hash, @u5.hash)
    assert_not_equal(@u4.hash, @u3.hash)    
  end
end
