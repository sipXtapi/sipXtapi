/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/test/com/pingtel/pds/sds/test/TestPhoneUpdate.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.sds.test;

import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.UnknownHostException;
import java.rmi.Naming;
import java.rmi.NotBoundException;
import java.rmi.RMISecurityManager;
import java.rmi.RemoteException;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.sds.ProfileListener;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class TestPhoneUpdate {

    /** This RMI listener (server) in the SDS waits for Rendered Profile Updates */
    private ProfileListener m_profileListener = null;

    /** Main entry point to the test code */
    public static void main(String[] args) {
        if (args.length < 1) {
            System.out.println("USAGE: java com.pingtel.pds.sds.test.TestPhoneUpdate <macaddress> [profileBaseURL]");
            System.out.println("ex:    java com.pingtel.pds.sds.test.TestPhoneUpdate 00d01e001a01 http://sipuaconfig:80/");
            System.exit(-1);
        }

        System.setProperty("java.rmi.server.codebase",
            "file:/main/product/sw/pds/sds/jbuilder/classes/ " +
            "file:/main/product/sw/pds/sds/build/classes/ " +
            "file:/main/product/sw/pds/pgs/build/common.jar");

        System.setSecurityManager(new RMISecurityManager());

        try {
            TestPhoneUpdate testPhoneUpdate1 = new TestPhoneUpdate();
            String macAddress = args[0];
            String baseURL = InetAddress.getLocalHost().getHostAddress();
            if (args.length > 1)
                baseURL = args[1];
            testPhoneUpdate1.updatePhoneProfile (
                PDSDefinitions.PROF_TYPE_PHONE,
                macAddress, "sip:1129@10.1.1.186",
                PDSDefinitions.MODEL_HARDPHONE_CISCO_7960,
                "Pingtel/Xpressa/00d01e00193b/pinger-config", 1);
        } catch (UnknownHostException ex) {
            System.out.println("Cannot look up local UP Address, exiting");
            System.exit(-1);
        } catch (RemoteException ex) {
            System.out.println("Unable to connect to SDS service, exiting");
            System.exit(-1);
        } catch (PDSException ex) {
            System.out.println("Unable to connect to SDS service, exiting");
            System.exit(-1);
        }
    }

    /** Constructor that looks up the Remote RMI server interface proxy */
    public TestPhoneUpdate() throws PDSException {
        // Attempt to send the document root to SDS, not critical
        // if it does not work at this stage as the SDS may be down
        try {
            m_profileListener =
                (ProfileListener) Naming.lookup(
                    "rmi://localhost:2001/SDSProfileListener" );
        } catch ( MalformedURLException ex ) {
            throw new PDSException ("Warning: bad url: rmi://localhost:2001/SDSProfileListener" + ex.getMessage());
        } catch ( NotBoundException ex ) {
            throw new PDSException ("Warning: SDS Service not started... please start it" + ex.getMessage());
        } catch ( RemoteException ex ) {
            throw new PDSException ("Error connecting to SDS: " + ex.getMessage(), ex );
        }
    }

    /**
     * This sends the 4 parameters over RMI to the SDS, the SDS will look up
     * its enrollment database for the mac address and it it si there it will
     * notify the phone of the updated profile
     */
    public void updatePhoneProfile ( int profileType,
                                     String macAddress,
                                     String deviceURL,
                                     String model,
                                     String url,
                                     int sequenceNumber ) throws RemoteException {
        m_profileListener.
            updatePhoneProfile( profileType,
                                macAddress,
                                deviceURL,
                                model,
                                url,
                                sequenceNumber );
    }
}