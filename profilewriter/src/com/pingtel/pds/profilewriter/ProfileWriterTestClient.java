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

package com.pingtel.pds.profilewriter;

import java.io.IOException;
import java.rmi.Naming;
import java.rmi.RMISecurityManager;
import java.rmi.RemoteException;
import java.util.HashMap;
import java.util.Properties;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;

public class ProfileWriterTestClient {
    /** This is a simple tester application that exercises the JNI server */
    public static void main(String[] argv) {
        String homedir = System.getProperty("user.home");
        String serverName = "";
        System.setSecurityManager(new RMISecurityManager());
        if (argv.length != 1) {
            try {
                serverName = java.net.InetAddress.getLocalHost().getHostName();
            } catch(Exception e) {
                e.printStackTrace();
            }
        } else {
            serverName = argv[0];
        }

        ProfileWriterTestClient client = new ProfileWriterTestClient();
        client.runTest(serverName);

    }

    private void runTest(String serverName) {
        if (serverName == "") {
            System.out.println(
                "usage: com.pingtel.pds.profilewriter.ProrileWriterTestClient " +
                "<IP address of host running RMI server>");
            System.exit(0);
        }

        // test tftp protocol
/*
        try {
            java.io.FileOutputStream output = new java.io.FileOutputStream( "c:/syncinfo.xml" );
            FileTransport.getInstance().downloadFile(FileTransport.PROTOCOL_TFTP, "syncinfo.xml", "localhost", false, output);
            output.close();

            java.io.FileInputStream input = new java.io.FileInputStream( "c:/SubscriptionListener.java" );
            FileTransport.getInstance().uploadFile(FileTransport.PROTOCOL_TFTP, "SubscriptionListener.java", "localhost", false, input);
        } catch (Exception ex) {
            System.out.println(ex.getMessage());
        }
*/
        try {
            // bind server object to object in client
            ProfileWriter myProfileWriter =
                (ProfileWriter)Naming.
                    lookup("rmi://" + serverName + ":2001/ProfileWriter");

            // Create an example SIP<macaddress>.cnf for the cisco phone
            HashMap ciscoConfigValues = new HashMap();

            ciscoConfigValues.put("proxy_backup", "7.8.9.0");
            ciscoConfigValues.put("proxy_backup_port", "5062");

            byte[] ciscoProfileBytes = createProfileByteStream(
                PDSDefinitions.MODEL_HARDPHONE_CISCO_7960,
                ciscoConfigValues );

            String ciscoMacAddress = "003094C41B2A";

//            myProfileWriter.updateDeviceProfile(
//                PDSDefinitions.PROF_TYPE_PHONE,             // Cisco only support the device config
//                ciscoMacAddress,                            // The CISCO Phone MAC address
//                "1129@10.1.1.186",                          // Cisco URL in Proxy
//                "Cisco",                                    // Manufacturer
//                PDSDefinitions.MODEL_HARDPHONE_CISCO_7960,  // Model
//                "SIP" + ciscoMacAddress + ".cnf",           // Leaf Node
//                ciscoProfileBytes, 1 );

            // Create a user config for the Xpressa phone
            HashMap xpressaConfigValues = new HashMap();
            xpressaConfigValues.put("PHONESET_DND_METHOD", "FORWARD_ON_BUSY");
            xpressaConfigValues.put("PHONESET_HTTP_AUTH_DB.admin", "fbf67629d04a8575645528409eea28a5");
            xpressaConfigValues.put("PHONESET_HTTP_PORT", "80");
            xpressaConfigValues.put("PHONESET_TELNET_ACCESS", "DISABLE");
            xpressaConfigValues.put("SIP_AUTHENTICATE_SCHEME", "NONE");
            xpressaConfigValues.put("SIP_CONFIG_SERVER_ADDRESS", "10.1.1.152");


            byte[] xpressaProfileBytes = createProfileByteStream(
                PDSDefinitions.MODEL_HARDPHONE_CISCO_7960,
                xpressaConfigValues );

//            String xpressaMacAddress = "00d01e00193b";
//
//            myProfileWriter.updatePhoneProfile(
//                PDSDefinitions.PROF_TYPE_USER,              // Cisco only support the device config
//                ciscoMacAddress,                            // The CISCO Phone MAC address
//                "1167@10.1.1.186",                          // Cisco URL in Proxy
//                "Cisco",                                    // Manufacturer
//                PDSDefinitions.MODEL_HARDPHONE_CISCO_7960,  // Model
//                "Pingtel/xpressa/" +
//                xpressaMacAddress +
//                "/user-config",                             // Leaf Node
//                xpressaProfileBytes, 1 );

            /*
            public void updateDeviceProfile ( int profileType,
                                              String macAddress,
                                              String deviceURL,
                                              String manufacturer,
                                              String model,
                                              String leafFileName,
                                              byte[] profile,
                                              int sequenceNumber )
                throws RemoteException, PDSException;
        */


//            for ( int i = 0 ; i < 10000; ++i ) {
//                myProfileWriter.updateDeviceProfile(PDSDefinitions.PROF_TYPE_USER,
//                        "MAC1000" + i,
//                        null,
//                        "Pingtel",
//                        "xpressa_strongarm_vxworks",
//                        "pinger-config",
//                        new byte [] { 1,2,3,4,5,6 },
//                        1);
//            }

              for ( int i = 0 ; i < 1; i += 1 ) {
                new Thread ( new MessageSender ( myProfileWriter, i )).start();

              }


            System.out.println ("Shutting down ProfileWriter server");
            // Shutdown the server object
            // myProfileWriter.shutdown();
        } catch(Exception e) {
            System.out.println("Exception occured: " + e);
            System.exit(0);
        }
        System.out.println("Test Complete");
    }


    /** make a byte stream from the properties file */
    static byte[] createProfileByteStream (String vendor, HashMap nameValuePairs)
        throws IOException
    {
        final byte[] myBytes = {1,2,3,4,5,6,7,8,9,0};
        return myBytes;
    }

    private class MessageSender implements Runnable {

        ProfileWriter m_connection;
        int m_min;

        public MessageSender ( ProfileWriter connection, int min ) {
            this.m_connection = connection;
            m_min = min;
        }

        public void run () {
            for ( int i = m_min ; i < 2; ++i ) {
                try {
                    m_connection.updateDeviceProfile(PDSDefinitions.PROF_TYPE_PHONE,
                            "MAC1000" + i,
                            null,
                            "Cisco",
                            PDSDefinitions.MODEL_HARDPHONE_CISCO_7960,
                            "CISCOFile" + i + ".txt",
                            new byte [] { 1,2,3,4,5,6 },
                            1);

                    /**
                     *  public void updateDeviceProfile ( int profileType,
                                      String macAddress,
                                      String deviceURL,
                                      String manufacturer,
                                      String model,
                                      String leafFileName,
                                      byte[] profile,
                                      int sequenceNumber )
                     */
                } catch (RemoteException e) {
                    e.printStackTrace();  //To change body of catch statement use Options | File Templates.
                } catch (PDSException e) {
                    e.printStackTrace();  //To change body of catch statement use Options | File Templates.
                }
            }
        }
    }
}