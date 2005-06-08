/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig;

/**
 * To be run from ANT scripts when setting up database
 */
public class ClearDb {

    public static void main(String[] args) {        
        try {
            TestHelper.cleanInsert("ClearDb.xml");
            // system exit is nec. or ant will not return
            System.exit(0);
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
}
