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

package org.sipfoundry.telephony ;

import javax.telephony.* ;
import javax.telephony.phone.* ;

public class PtTerminalConnectionEvent extends PtCallEvent implements TerminalConnectionEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to the terminal connection tied to this event */
    protected TerminalConnection m_terminalConnection ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * constructor taking originating terminal and terminal connection.
     * This is an immutable object and thus no set accessors are defined.
     */
    public PtTerminalConnectionEvent(   Call                call,
                                        TerminalConnection  terminalConnection)
    {
        super(call, LS_UNKNOWN) ;
        m_terminalConnection = terminalConnection ;
    }


    /**
     * JNI constructor taking originating call and terminal connection.
     * This is an immutable object and thus no set accessors are defined.
     */
    public PtTerminalConnectionEvent(   long lCall,
                                        long lTerminalConnection,
                                        int  iLocalState)
    {
        super(lCall, iLocalState) ;
        m_terminalConnection = PtTerminalConnection.fromNativeObject(lTerminalConnection) ;
    }



//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public TerminalConnection getTerminalConnection()
    {
        return m_terminalConnection ;
    }



    /**
     * Convenience/efficient method that allows developers to query whether
     * this event is delivered to the local terminal connection.  The reduces
     * the need to get and query the TC directly.
     */
    public boolean isLocalTerminalConnection()
    {
        boolean bIsLocal = false ;

        if (getLocalState() != LS_UNKNOWN)
        {
            bIsLocal = (getLocalState() == LS_LOCAL) ;
        }
        else
        {
            PtTerminalConnection tc = (PtTerminalConnection) getTerminalConnection() ;
            if ((tc != null) && tc.isLocal())
                bIsLocal = true ;
        }
        return bIsLocal ;
    }
}
