/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/cmd/UpgradeCiscoCommand.java#6 $
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

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class UpgradeCiscoCommand implements Command {
    CiscoReceiver m_receiver;
    String m_macAddress;
    String m_deviceURL;

    /**
     * Upgrading the Cisco Command needs a Cisco Receiver amd the SIP deviceURL
     * Cisco do not understand mac addresses
     */
    public UpgradeCiscoCommand ( CiscoReceiver receiver,
                                 String macAddress,
                                 String deviceURL ) {
        m_receiver = receiver;
        m_macAddress = macAddress;
        m_deviceURL = deviceURL;
    }

    /**
     * This functionality does not really work in a reliable manner
     * if this command is received we should reset the phone where
     * it can pick up its new kernel
     */
    public void execute() throws CommandException {
        m_receiver.reset( m_macAddress, m_deviceURL );
    }
}