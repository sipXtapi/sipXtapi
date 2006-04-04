#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# Gateway models a gateway defined in the SIPXCONFIG database.
class Gateway < ActiveRecord::Base
  set_primary_key('gateway_id')
  set_table_name('gateway')
end
