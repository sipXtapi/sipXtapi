#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'test/unit'

$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

# application requires
require 'utils/socket_utils'


class SocketUtilsTest < Test::Unit::TestCase

  def test_strip_v4_port
    assert_equal('1.1.1.1', SocketUtils.strip_v4_port('1.1.1.1:123'))
    assert_equal('1.1.1.1', SocketUtils.strip_v4_port('1.1.1.1'))
    assert_equal('domain.com', SocketUtils.strip_v4_port('domain.com'))
  end

end
