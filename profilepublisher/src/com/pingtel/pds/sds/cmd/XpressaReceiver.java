/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/cmd/XpressaReceiver.java#6 $
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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.io.IOException;
import java.net.InetAddress;

import org.apache.log4j.Category;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.sds.sip.SipNotifyHelper;
import com.pingtel.pds.sds.sip.SipException;
import com.pingtel.pds.sds.sip.SipSessionContext;
import com.pingtel.pds.sds.sip.SipCheckSyncHelper;

/**
 * Title:        XpressaReceiver
 * Description:  This object is used to sent both resets and profiles to the Phone
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author:      John P. Coffey
 * @version 1.0
 */
public class XpressaReceiver {

    /** Singleton */
    private static XpressaReceiver g_instance = null;

    private Category m_logger = Category.getInstance( "pds" );


    /** singleton Accessor */
    public static XpressaReceiver getInstance() {
        if (g_instance == null) {
            g_instance = new XpressaReceiver ();
        }
        return g_instance;
    }

    /**
     * This method is responsible for using SIP to upgrade the software.
     * The method is responsible for Upgrading the software on a Pingtel
     * OPnly phone.  The Phone uses a catalog to intelligently
     *
     * @param macAddress
     * @param deviceURL
     * @param currentVersion
     * @param targetVersion
     * @param catalogURL
     *
     * @exception CommandException
     */
    public void upgrade ( String macAddress,
                          String deviceURL,
                          String currentVersion,
                          String targetVersion,
                          String catalogURL )
        throws CommandException {
    }

    /**
     *
     * @param macAddress
     * @param deviceURL
     * @param requestedProfiles
     *
     * @exception CommandException
     */
    public void sendProfiles ( String macAddress,
                               String deviceURL,
                               String fromURL,
                               String callID,
                               int outputCSeq,
                               String documentRoot,
                               ArrayList requestedProfiles )
        throws CommandException {
        try {
            m_logger.debug ( "about to send profiles to MAC: " +  macAddress +
                                " with callID: " + callID );
            SipNotifyHelper.getInstance().notify(
                macAddress, deviceURL,
                fromURL, callID,
                outputCSeq, documentRoot,
                requestedProfiles);
        } catch ( SipException ex ) {
            m_logger.error( "error in sendProfiles: " + ex.toString(), ex );
            throw new CommandException ( ex.getMessage() );
        }
    }


     /**
     * Resets the Phone, this method uses SIP to send the phone a Check-Sync Command
     */
    public void reset ( String fromAddress,
                        String toAddress,
                        String callID,
                        String contactAddress,
                        int sequenceNumber )
        throws CommandException {
        try {
            SipCheckSyncHelper.getInstance().
                sendCheckSync( fromAddress,
                               toAddress,
                               callID,
                               contactAddress,
                               sequenceNumber );
        }
        catch (IOException ex) {
            m_logger.error( "error in reset: " + ex.toString(), ex );
            throw new CommandException(ex.getMessage());
        }
    }
}