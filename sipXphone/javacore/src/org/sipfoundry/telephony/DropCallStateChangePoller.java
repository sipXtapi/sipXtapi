/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/telephony/DropCallStateChangePoller.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.telephony;

import javax.telephony.* ;
import javax.telephony.callcontrol.* ;

import org.sipfoundry.sip.* ;
import org.sipfoundry.sip.event.* ;

import org.sipfoundry.stapi.PSession ;

/**
 * This state change poller poll until all of remote connections are in a
 * final state.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class DropCallStateChangePoller extends AbstractStateChangePoller
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected PtCall     m_call ;       // JTAPI Call

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs the Held call state change poller with the required JTAPI
     * call object and optional session.
     */
    public DropCallStateChangePoller(PtCall call)
    {
        super() ;

        m_call = call ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Determine if there are any live connections.  If so return FALSE and
     * continuing polling, otherwise return TRUE.
     */
    public boolean poll()
    {
        boolean bLiveConnections = false ;

        synchronized (m_call)
        {
            Connection connections[] = m_call.getConnections() ;
            if (connections != null)
            {
                // Start with connection index 1, 0 is always the local connection
                for (int i=1; i<(connections.length) && (bLiveConnections == false); i++)
                {
                    TerminalConnection tcs[] = connections[i].getTerminalConnections() ;
                    if (tcs != null)
                    {
                        for (int j=0;j<tcs.length; j++)
                        {
                            int iTCState = tcs[j].getState() ;
                            if ((iTCState != CallControlTerminalConnection.DROPPED) &&
                                    (iTCState != CallControlTerminalConnection.UNKNOWN) &&
                                    (iTCState != CallControlTerminalConnection.IDLE))
                            {
                                bLiveConnections = true ;
                                break ;
                            }

                            // NOTE: You should not need to perform this final step,
                            // however, the lower layers are marking the remote TC
                            // as DROPPED while the connection is still established.
                            int iConnState = connections[i].getState() ;
                            if ((iConnState != CallControlConnection.DISCONNECTED) &&
                                    (iConnState != CallControlConnection.FAILED))
                            {
                                bLiveConnections = true ;
                                break ;
                            }
                        }
                    }
                }
            }
        }
        return !bLiveConnections ;
    }


    /**
     * Clean up any resource for the poller.  In this implementation, we may
     * need to remove our incoming message listener from the Sip User Agent.
     */
    public void cleanup()
    {

    }
}
