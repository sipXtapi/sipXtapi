/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/test/com/pingtel/pds/sds/test/TestProfileListener.java#4 $
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

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.sds.ProfileListener;
import junit.framework.Assert;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

import java.rmi.Naming;
import java.rmi.RemoteException;

/**
 * TestProfileListener performs black-box tests on the Profile Writer's interface
 * to the SDS.   It uses the same interface as the Profile Writer, the RMI connection,
 * to send various primitives to the SDS.
 *
 * @author IB
 */
public class TestProfileListener extends TestCase {

    private static final String CISCO_SERIAL_NUMBER = "000000222222";
    private static final String PINGTEL_SERIAL_NUMBER = "000000111111";

    private ProfileListener m_rmiStub; // connection used to communicate w/ SDS

    public TestProfileListener ( String name ) {
        super ( name );
    }

    public static Test suite() {
        return new TestSuite( TestProfileListener.class );
    }

    protected void setUp () {
        try {
            m_rmiStub = (ProfileListener)
                    Naming.lookup( "rmi://localhost:2001/SDSProfileListener");
        }
        catch ( Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }



    public void testResetPingtelDevice () {
        try {
            m_rmiStub.resetPhone( PINGTEL_SERIAL_NUMBER, null );
        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }


    public void testReset79xxDevice () {
        try {
            m_rmiStub.resetPhone( CISCO_SERIAL_NUMBER, "sip:foobar@pingtel.com" );
        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }


    public void testUpdatePingtelDeviceProfile () {
        try {
            updatePingtelDevice (   PDSDefinitions.PROF_TYPE_PHONE,
                                    "http://pingtel/xpressa/" + PINGTEL_SERIAL_NUMBER + "/pinger-config");
        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }


    public void testUpdatePingtelUserProfile () {
        try {
            updatePingtelDevice (   PDSDefinitions.PROF_TYPE_USER,
                                    "http://pingtel/xpressa/" + PINGTEL_SERIAL_NUMBER + "/user-config");
        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }


    public void testUpdatePingtelApplicationProfile () {
        try {
            updatePingtelDevice (   PDSDefinitions.PROF_TYPE_APPLICATION_REF,
                                    "http://pingtel/xpressa/" + PINGTEL_SERIAL_NUMBER + "/app-config");
        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }

    public void testUpdate79xxDeviceProfile () {
        try {
            update79xxDevice ( PDSDefinitions.PROF_TYPE_PHONE );
        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }


    public void testUpdate79xxUserProfile () {
        try {
            update79xxDevice ( PDSDefinitions.PROF_TYPE_USER );
        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }


    public void testSetDocumentRoot () {
        try {
            m_rmiStub.setDocumentRoot( "file:/main/product/sw/pds");
        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }


    private void updatePingtelDevice ( int profileType, String profileURL )
            throws RemoteException {

        m_rmiStub.updatePhoneProfile(   profileType,
                                        PINGTEL_SERIAL_NUMBER,
                                        null,
                                        PDSDefinitions.MODEL_HARDPHONE_XPRESSA,
                                        profileURL,
                                        1 );
    }


    private void update79xxDevice ( int profileType )
            throws RemoteException {

        m_rmiStub.updatePhoneProfile(   profileType,
                                        CISCO_SERIAL_NUMBER,
                                        "sip:someuser@somedomain.com",
                                        PDSDefinitions.MODEL_HARDPHONE_CISCO_7960,
                                        null,
                                        1 );
    }

}
