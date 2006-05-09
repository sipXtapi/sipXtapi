#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'logger'


# Rails somehow modifies the default logging format so that all you get is the
# message text.  This class assumes that the modification is in effect.
class SipxLogger < Logger
end
