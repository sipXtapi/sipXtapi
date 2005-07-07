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

public class PtConnectionEvent extends PtCallEvent implements ConnectionEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to the terminal connection tied to this event */
    protected Connection m_connection ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * constructor taking originating terminal and terminal connection.
     * This is an immutable object and thus no set accessors are defined.
     */
    public PtConnectionEvent(   Call        call,
                                Connection  connection)
    {
        super(call, LS_UNKNOWN) ;
        m_connection = connection ;
    }


    /**
     * constructor taking originating terminal and terminal connection.
     * This is an immutable object and thus no set accessors are defined.
     */
    public PtConnectionEvent(   long lCall,
                                long lConnection,
                                int  iLocalState)
    {
        super(lCall, iLocalState) ;
        m_connection = PtConnection.fromNativeObject(lConnection) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public Connection getConnection()
    {
        return m_connection ;
    }


    /**
     * @deprecated do not expose
     */
    public boolean isLocalConnection()
    {
        boolean bIsLocal = false ;

        if (getLocalState() != LS_UNKNOWN)
        {
            bIsLocal = (getLocalState() == LS_LOCAL) ;
        }
        else
        {
            TerminalConnection tcs[] = m_connection.getTerminalConnections() ;
            if ((tcs.length > 0) && ((PtTerminalConnection) tcs[0]).isLocal())
            {
                bIsLocal = true ;
            }
        }

        return bIsLocal ;
    }
}
