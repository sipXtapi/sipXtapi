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

package com.pingtel.commserver.utility;

/**
 * A basic class for logging information. 
 * Right now, it just writes to standard out.
 * This can be extended to write to approriate log files, etc.
 * 
 * @author Harippriya Sivapatham
 */
public class LogUtility {
//////////////////////////////////////////////////////////////////////////////
// Attributes
////


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    
    public LogUtility()
    {
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Writes the text to the log file.
     * 
     */ 
    public void writeToLog( String logText)
    {
        // At this point, all info is written to the standard out.
        // We can extend this to write to appropriate log file based on the server.
        System.out.println( logText );
        return;
    }
}

