/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/common/RMIConnectionManager.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.pgs.common;

import java.rmi.Naming;
import java.rmi.RMISecurityManager;
import java.util.Hashtable;

import com.pingtel.pds.sds.ProfileListener;

/**
 * SDSWriterConnection is a singleton which controls access to the RMI
 * mInstance to the SDS process.   It is called by the PGS to
 * send commands (currently just profile projections) to the ProfileWriter.
 *
 */
public class RMIConnectionManager {

//////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final String CONNECT_ERROR_MESSAGE =
        "Unable to make a mInstance to the SDS - make sure that " +
        "it RMIRegistry and the ProfileListener are running. Details: ";


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    // singleton instance
    private static RMIConnectionManager mInstance = new RMIConnectionManager ();

    private Hashtable mConnectionMap = new Hashtable();


//////////////////////////////////////////////////////////////////////////
// Construction
////
    private RMIConnectionManager () {
        System.setSecurityManager(new RMISecurityManager());
    }


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * accessor to Singleton instance
     * @return returns Singleton instance
     */
    public static RMIConnectionManager getInstance() {
        return mInstance;
    }

    /**
     * getConnection produces RMI client stubs for the caller.
     *
     * @param URL RMI URL for the SDS you want to connect to
     * @return ProfileListener RMI client stub for the the given
     * RMI URL
     * @throws Exception for all errors
     */
    public Object getConnection (String URL) throws Exception {
        if (!mConnectionMap.containsKey(URL)){
            Object o = makeConnection(URL);
            mConnectionMap.put(URL, o);
        }
        return mConnectionMap.get(URL);
    }


    public Object reestablishConnection(String URL) throws Exception {
        if (mConnectionMap.containsKey(URL)){
             mConnectionMap.remove(URL);
        }

        return getConnection(URL);
    }


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    private Object makeConnection ( String URL) throws Exception {
        try {
            Object o =  Naming.lookup( URL );
            return o;
        }
        catch ( Exception ex ) {
            throw new Exception  ( CONNECT_ERROR_MESSAGE + ex.toString() );
        }
    }


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////

//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}