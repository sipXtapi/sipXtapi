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

import com.pingtel.pds.sds.cache.SessionCache;

public class RestartXpressaCommand implements Command {
    private XpressaReceiver m_receiver;
    private String m_fromAddress;
    private String m_toAddress;
    private String m_callID;
    private String m_contactAddress;
    private int m_sequenceNumber;

    /** Constructor */
    public RestartXpressaCommand ( XpressaReceiver receiver,
                                   String fromAddress,
                                   String toAddress,
                                   String callID,
                                   String contactAddress,
                                   int sequenceNumber ) {
        m_receiver = receiver;
        m_fromAddress = fromAddress;
        m_toAddress = toAddress;
        m_callID = callID;
        m_contactAddress = contactAddress;
        m_sequenceNumber = sequenceNumber;
    }


    /**
     * This method simply sends a check-sync notify message to the phone
     * the profile writer ensures that the
     */
    public void execute() throws CommandException {
        m_receiver.reset ( m_fromAddress,
                           m_toAddress,
                           m_callID,
                           m_contactAddress,
                           m_sequenceNumber);
    }
}
