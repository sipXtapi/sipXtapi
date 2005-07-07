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


package com.pingtel.pds.profilewriter.test;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.profilewriter.ProfileWriter;
import junit.framework.Assert;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

import java.io.File;
import java.rmi.Naming;
import java.rmi.RemoteException;

/**
 * TestPGSInterface performs black-box tests on the Profile Writer.   It uses
 * the same interface as the PGS, the RMI connection, to send various
 * primitives to the Profile Writer.
 *
 * Note: currently this only works in the development perforce directory
 * structure.   If you try to run this in production (if you can?) then
 * it will fail.
 *
 * @author IB
 */
public class TestPGSInterface extends TestCase {

    private static final String DATA_DIR = "..\\data\\";
    private static final String DOCROOT = DATA_DIR + "docroot\\";
    private static final String PARTIAL_DIR = DATA_DIR + "partial\\";
    private static final String TFTP_DIR = DATA_DIR + "tftproot\\";

    // #millis to wait to allow the profile writer time to actually carry out its
    // requested task.
    private static final int SLEEP_INTERVAL = 2000;

    private static final String CISCO_SERIAL_NUMBER = "000000222222";
    private static final String PINGTEL_SERIAL_NUMBER = "000000111111";

    private static byte [] PROFILE_CONTENTS; // holder for fake profile contents

    private ProfileWriter m_rmiStub; // connection used to communicate w/ profile writer

    public TestPGSInterface ( String name ) {
        super ( name );
        String profileString = "Profile String";
        PROFILE_CONTENTS = profileString.getBytes();
    }

    public static Test suite() {
        return new TestSuite( TestPGSInterface.class );
    }

    protected void setUp () {
        try {
            m_rmiStub = (ProfileWriter) Naming.lookup( "rmi://localhost:2001/ProfileWriter");
        }
        catch ( Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }


    /**
     * creates and deletes a device profile for a Pingtel xpressa device
     */
    public void testPingtelCreateAndDeleteDevice () {

        try {
            createPingtelProfile( PDSDefinitions.PROF_TYPE_PHONE, "pinger-config");
            checkPingtelProfileExists( "pinger-config");
            deletePingtelProfile( PDSDefinitions.PROF_TYPE_PHONE, "pinger-config" );
        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }


    /**
     * creates and deletes a user profile for a Pingtel xpressa device
     */
    public void testPingtelCreateAndDeleteUser () {

        try {
            createPingtelProfile( PDSDefinitions.PROF_TYPE_USER, "user-config");
            checkPingtelProfileExists( "user-config");
            deletePingtelProfile( PDSDefinitions.PROF_TYPE_USER, "user-config" );

        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }


    /**
     * creates and deletes a application profile for a Pingtel xpressa device
     */
    public void testPingtelCreateAndDeleteApplication () {

        try {
            createPingtelProfile( PDSDefinitions.PROF_TYPE_APPLICATION_REF, "app-config");
            checkPingtelProfileExists( "app-config");
            deletePingtelProfile( PDSDefinitions.PROF_TYPE_APPLICATION_REF, "app-config" );
        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }


    /**
     * creates and deletes a device profile for a Cisco 7960 device
     */
    public void test79xxCreateAndDeleteDevice( ) {
        try {
            create79xxProfile ( PDSDefinitions.PROF_TYPE_PHONE, "device-settings.txt" );
            checkCiscoProfileExists ( "device-settings.txt" );

            delete79xxProfile ( PDSDefinitions.PROF_TYPE_PHONE, "device-settings.txt" );
        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }


    /**
     * creates and deletes a user profile for a Cisco 7960 device
     */
    public void test79xxCreateAndDeleteUser( ) {
        try {
            create79xxProfile ( PDSDefinitions.PROF_TYPE_PHONE, "user-settings.txt" );
            checkCiscoProfileExists ( "user-settings.txt" );

            delete79xxProfile ( PDSDefinitions.PROF_TYPE_PHONE, "user-settings.txt" );
        }
        catch (Exception e) {
            e.printStackTrace();
            Assert.fail( e.getMessage() );
        }
    }




    private void createPingtelProfile( int profileType, String filename )
            throws RemoteException, PDSException {

        m_rmiStub.updateDeviceProfile (
                    profileType,
                    PINGTEL_SERIAL_NUMBER,
                    filename,
                    "pingtel",
                    PDSDefinitions.MODEL_HARDPHONE_XPRESSA,
                    filename,
                    PROFILE_CONTENTS,
                    1 );
    }


    private void deletePingtelProfile( int profileType, String fileName )
            throws RemoteException, PDSException {

        m_rmiStub.removeDeviceProfile (
                    profileType,
                    PINGTEL_SERIAL_NUMBER,
                    null,
                    "pingtel",
                    PDSDefinitions.MODEL_HARDPHONE_XPRESSA,
                    fileName );
    }


    private void create79xxProfile( int profileType, String filename )
            throws RemoteException, PDSException {

        m_rmiStub.updateDeviceProfile (
                    profileType,
                    CISCO_SERIAL_NUMBER,
                    null,
                    "cisco",
                    PDSDefinitions.MODEL_HARDPHONE_CISCO_7960,
                    filename,
                    PROFILE_CONTENTS,
                    1 );
    }


    private void delete79xxProfile( int profileType, String fileName )
            throws RemoteException, PDSException {

        m_rmiStub.removeDeviceProfile (
                    profileType,
                    CISCO_SERIAL_NUMBER,
                    null,
                    "cisco",
                    PDSDefinitions.MODEL_HARDPHONE_CISCO_7960,
                    fileName );
    }


    private void checkPingtelProfileExists( String leafFilename ) throws InterruptedException {
        Thread.sleep(SLEEP_INTERVAL);

        String baseName = DOCROOT +  "pingtel\\" + PDSDefinitions.MODEL_HARDPHONE_XPRESSA +
                "\\" + PINGTEL_SERIAL_NUMBER + "\\";


        testFileExists(baseName + leafFilename );
    }


    private void checkCiscoProfileExists( String leafFilename ) throws InterruptedException {
        Thread.sleep(SLEEP_INTERVAL);
        String baseName = PARTIAL_DIR + "cisco\\7960\\" + CISCO_SERIAL_NUMBER +  "\\";
        testFileExists ( baseName + leafFilename);

        baseName = TFTP_DIR;
        testFileExists ( baseName + "SIP" + CISCO_SERIAL_NUMBER + ".cnf" );
    }

    private void testFileExists(String filename) {
        File profile1 = new File (  filename );
        Assert.assertTrue( profile1.exists() );
    }

}
