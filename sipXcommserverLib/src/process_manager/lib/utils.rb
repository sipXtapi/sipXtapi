#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# system requires

# application requires


class Utils
  # If the process identified by the keyword is running, then return its PID.
  # Otherwise return nil.
  # Grep for the keyword in the output of the ps command to accomplish this.
  # Use "grep -v defunct" to eliminate all lines for defunct processes.
  def self.get_process_pid(keyword)
    pid = nil   # return value
    shellReturn = `ps -fC #{keyword} | grep -v defunct | grep #{keyword}`
    puts shellReturn
    shellReturn = shellReturn.strip
    re = Regexp.new('\A\D+\s+(\d+)\s+.*' + keyword + '.*')
    if re =~ shellReturn
      pid = $1.to_i
    end
    pid
  end
end
