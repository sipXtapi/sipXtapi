/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/telephony/HeldCallStateChangePoller.java#2 $
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
 * This state change poller poll until all of remote connections are in the
 * HELD (on hold) state.  Additionally, this method optionally adds a
 * sip message listener and looks for 400 class responses.
 *
 * This implementation is a dirty because it pulls in classes from the
 * org.sipfoundry.sip package  and org.sipfoundry.stapi packages.  This is needed to
 * avoid costly calls to the lower layers.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class HeldCallStateChangePoller extends AbstractStateChangePoller
        implements NewSipMessageListener
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected PtCall     m_call ;       // JTAPI Call
    protected PSession[] m_sessions ;   // PTAPI Sessions

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs the Held call state change poller with the required JTAPI
     * call object and optional session.
     */
    public HeldCallStateChangePoller(PtCall call, PSession sessions[])
    {
        super() ;

        m_call = call ;
        m_sessions = sessions ;

        if (m_sessions != null)
        {
            for (int i=0; i<m_sessions.length; i++)
            {
                if (m_sessions[i] != null)
                {
                    SipMessageFilterCriteria criteria = new SipMessageFilterCriteria((SipSession) m_sessions[i]) ;
                    criteria.restrictMethod("INVITE") ;
                    criteria.restrictType(SipMessageFilterCriteria.RESPONSE) ;

                    SipUserAgent.getInstance().addIncomingMessageListener(criteria, this) ;
                }
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Determine if there are any unheld connections.  If so return FALSE and
     * continuing polling, otherwise return TRUE.
     */
    public boolean poll()
    {
        boolean bUnheldConnections = false ;

        synchronized (m_call)
        {
            Connection connections[] = m_call.getConnections() ;
            // Start with connection index 1, 0 is always the local connection
            for (int i=1;(i<connections.length) && (bUnheldConnections == false); i++)
            {
                TerminalConnection tcs[] = connections[i].getTerminalConnections() ;

                for (int j=0;j<tcs.length; j++)
                {
                    if (tcs[j].getState() == CallControlTerminalConnection.TALKING)
                    {
                        bUnheldConnections =  true ;
                        break ;
                    }
                }
            }
        }
        return !bUnheldConnections ;
    }


    /**
     * Clean up any resource for the poller.  In this implementation, we may
     * need to remove our incoming message listener from the Sip User Agent.
     */
    public void cleanup()
    {
        if (m_sessions != null)
        {
            SipUserAgent.getInstance().removeIncomingMessageListener(this) ;
        }
    }



    /**
     * Exposed as a side effect of implementing the NewSipMessageListener
     * interface.  Here, we are looking at inbound sip messages for 400+
     * class responses.  If found, we are aborting the poller with a result
     * of failed/false.
     */
    public void newMessage(NewSipMessageEvent event)
    {
        SipResponse response = (SipResponse) event.getMessage() ;
        int iResponse = response.getStatusCode() ;

        if ((iResponse == 486) || (iResponse == 480) || (iResponse == 400))
        {
            markCompleted(false) ;
        }
    }
}
