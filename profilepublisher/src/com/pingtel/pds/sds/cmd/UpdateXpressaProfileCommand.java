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

import com.pingtel.pds.sds.sip.SipProfileInfo;
import com.pingtel.pds.common.URLPinger;

import java.util.ArrayList;
import java.util.Iterator;
import java.net.URL;
/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class UpdateXpressaProfileCommand implements Command {
    private XpressaReceiver m_receiver;
    private ArrayList m_profileInfo;
    private String m_macAddress;
    private String m_deviceURL;
    private String m_fromURL;
    private String m_callID;
    private int m_outputCSeq;
    private String m_documentRoot;

    public UpdateXpressaProfileCommand( XpressaReceiver receiver,
                                        String macAddress,
                                        String deviceURL,
                                        String fromURL,
                                        String callID,
                                        int outputCSeq,
                                        String documentRoot,
                                        ArrayList profileInfo) {
        m_receiver = receiver;
        m_macAddress = macAddress;
        m_deviceURL = deviceURL;
        m_fromURL = fromURL;
        m_callID = callID;
        m_outputCSeq = outputCSeq;
        m_documentRoot = documentRoot;
        m_profileInfo = profileInfo;
    }

    public void execute() throws CommandException {
        m_receiver.sendProfiles(
            m_macAddress, m_deviceURL,
            m_fromURL, m_callID,
            m_outputCSeq, m_documentRoot,
            m_profileInfo);
    }
}