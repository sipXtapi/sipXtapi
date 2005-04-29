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
 * Title:        UpgradeXpressaCommand
 * Description:  Contains the state variables and the receiver required to
 *               upgrade a pingtel Xpressa Phone
 * Copyright:    Copyright (c) 2001
 * Company:      Pingrel Corp.
 * @author       John P. Coffey
 * @version 1.0
 */

public class UpgradeXpressaCommand implements Command {
    /** Command pattern receiver - does the real work */
    private XpressaReceiver m_receiver;

    /** the mac address of the phone */
    private String m_macAddress;

    /** the mac address of the phone */
    private String m_deviceURL;

    /** Pingtel Specific catalog URL for Smart Upgrades*/
    private String m_catalogURL;

    /**
     * the current software version, used by the UpgradeXpressaReceiver to
     * decide how to do the upgrade, if the current version does not support
     * some SIP features or requires an explicit reboot
     */
    private String m_currentVersion;

    /**
     * the target version of the software, this combined with the currentVersion
     * are used to look up the appropriate rules in the Catalog by the phone.
     */
    private String m_targetVersion;

    /** To send a command to an Xpressa phone we need to use the NOTIFY message */
    public UpgradeXpressaCommand ( XpressaReceiver receiver,
                                   String macAddress,
                                   String deviceURL,
                                   String currentVersion,
                                   String targetVersion,
                                   String catalogURL ) {
        m_receiver = receiver;
        m_macAddress = macAddress;
        m_deviceURL = deviceURL;
        m_currentVersion = currentVersion;
        m_targetVersion = targetVersion;
        m_catalogURL = catalogURL;
    }

    /** Forward the request onto the Specific receiver */
    public void execute() throws CommandException {
        // Do the actual phone upgrade by calling the receiver
        // and have it take care of the Protocol specific stuff
        m_receiver.upgrade( m_macAddress,
                            m_deviceURL,
                            m_currentVersion,
                            m_targetVersion,
                            m_catalogURL );
    }
}
