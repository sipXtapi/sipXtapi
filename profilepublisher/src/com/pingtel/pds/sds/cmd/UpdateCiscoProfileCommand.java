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

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class UpdateCiscoProfileCommand implements Command {
    private CiscoReceiver m_receiver;
    private String m_macAddress;
    private String m_deviceURL;    private String m_callID;

    public UpdateCiscoProfileCommand ( CiscoReceiver receiver,
                                       String macAddress,
                                       String deviceURL ) {
        m_receiver = receiver;
        m_macAddress = macAddress;
        m_deviceURL = deviceURL;
    }


    /**
     * This method simply sends a check-sync notify message to the phone
     * the profile writer ensures that the
     */
    public void execute() throws CommandException {
        m_receiver.reset ( m_macAddress, m_deviceURL );
    }
}