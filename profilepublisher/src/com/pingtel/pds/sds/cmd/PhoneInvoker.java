/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.sds.cmd;

public class PhoneInvoker {
    Command m_command;

    /**
     * The constructor contains a command object that has the state and arguments
     * required to make the entire transaction with the device
     */
    public PhoneInvoker ( Command command ) {
        m_command = command;
    }

    public void notifyPhone () throws CommandException {
        // Sends the command via the appropriate protocol to the device
        m_command.execute();
    }
}