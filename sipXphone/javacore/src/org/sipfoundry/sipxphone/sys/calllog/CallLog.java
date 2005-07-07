/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys.calllog ;

import java.net.* ;
import java.util.* ;
import javax.telephony.* ;


import org.sipfoundry.util.* ;
import org.sipfoundry.stapi.* ;

import org.sipfoundry.sipxphone.hook.* ;

import org.sipfoundry.sipxphone.sys.* ;

/**
 * NOTE: THIS CLASS IS HERE JUST FOR BACKWARD_COMPATIBILITY PURPPOSE.
 * IT IS REPLACED BY CallLogManager.
 *
 * CallLog used internally by the Xpressa.  Developers can query, filter,
 * remove, and add entries to the call log.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class CallLog
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** The default max number of entries the call log can/well hold */
    public static final int DEFAULT_MAX_ENTRIES = 25 ;

    /** The CallLog remember this max number of entries */
    public int MAX_ENTRIES = DEFAULT_MAX_ENTRIES ;

    /** Sorting order: Chronological */
    public static final int SORT_CHRONOLOGICAL = 0 ;
    /** Sorting order: Reverse Chronological */
    public static final int SORT_REVERSE_CHRONOLOGICAL = 1 ;
    /** Sorting order: Alphanumeric */
    public static final int SORT_ALPHANUMERIC = 2 ;
    /** Sorting order: Reverse Alphanumeric */
    public static final int SORT_REVERSE_ALPHANUMERIC = 3 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** list of call log entries */
    protected Vector m_vEntries = null ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Default no argument constructor.  Developers should obtain a call log
     * from the Shell.
     *
     * @deprecated
     */
    public CallLog()
    {
        PingerConfig config = PingerConfig.getInstance() ;
        String strMaxCallLogEntries = config.getValue("PHONESET_MAX_CALL_LOG_ENTRIES") ;
        if (strMaxCallLogEntries != null) {
            try {
                MAX_ENTRIES = Integer.parseInt(strMaxCallLogEntries) ;
            } catch (NumberFormatException e) {
                System.out.println("Invalid value set for max call log entries:") ;
                SysLog.log(e) ;
            }
        }
        loadCallLog() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get all of the call log entries.  This returns an array of all the
     * call log entries.
     *
     * @return array of all known call log entries with the most recent at
     *         position 0.
     */
    public synchronized CallLogEntry[] getAllEntries()
    {
        CallLogEntry rc[] = null ;

        rc = new CallLogEntry[m_vEntries.size()] ;
        m_vEntries.copyInto(rc) ;

        return rc ;
    }






//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     *
     */
    private void loadCallLog()
    {
        PropertyManager manager = PropertyManager.getInstance() ;
        try {
            m_vEntries = (Vector) manager.getObject("xpressa_calllog") ;
        } catch (Exception e) {
            m_vEntries = new Vector(MAX_ENTRIES + 1) ;
        }

        // Trim to our max entries
        while (m_vEntries.size() > MAX_ENTRIES) {
            m_vEntries.removeElement(m_vEntries.lastElement()) ;
        }
    }


    /**
     *
     */
    private void saveCallLog()
    {
        PropertyManager manager = PropertyManager.getInstance() ;
        try {
           manager.putObject("xpressa_calllog", m_vEntries) ;
        } catch (Exception e) {
            System.out.println("Unable to save call log: " + e) ;
            SysLog.log(e) ;
        }
    }


}