/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/cmd/RestartCiscoCommand.java#6 $
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
 * RestartCiscoCommand is a command pattern class which makes the coupling
 * between the restart Cisco Device primitive and the actual code which does
 * the work (the CiscoReceiver).
 *
 * @author ibutcher
 * @see CiscoReceiver#reset
 */
public class RestartCiscoCommand implements Command{

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////
    

    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    private CiscoReceiver mReceiver;
    private String mMACAdress;
    private String mDeviceURL;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////
    public RestartCiscoCommand (CiscoReceiver receiver,
                                String macAddress,
                                String deviceURL ) {
        mReceiver = receiver;
        mMACAdress = macAddress;
        mDeviceURL = deviceURL;
    }
    
    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    /**
     * This method simply sends a check-sync notify message to the phone
     */
    public void execute() throws CommandException {
        mReceiver.reset ( mMACAdress, mDeviceURL );
    }

    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////


    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////
}




