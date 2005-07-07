/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/telephony/ReleaseConnectionStateChangePoller.java#2 $
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
import org.sipfoundry.stapi.PAddress ;

/**
 * This state change poller poll until a remote connection is in the
 * TALKING (off hold) state.  Additionally, this method optionally adds a
 * sip message listener and looks for 400 class responses.
 *
 * This implementation is a dirty because it pulls in classes from the
 * org.sipfoundry.sip package  and org.sipfoundry.stapi packages.  This is needed to
 * avoid costly calls to the lower layers.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ReleaseConnectionStateChangePoller extends AbstractStateChangePoller
        implements NewSipMessageListener
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected PtCall    m_call ;      // JTAPI Call
    protected PAddress  m_address ;   // Connection we are looking at
    protected PSession  m_session ;   // PTAPI Session

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs the re;ease connection state change poller with the required
     * JTAPI call object, address, and optional session.
     */
    public ReleaseConnectionStateChangePoller(PtCall call, PAddress address, PSession session)
    {
        super() ;

        m_call = call ;
        m_address = address ;
        m_session = session ;

        if (m_session != null)
        {
            SipMessageFilterCriteria criteria = new SipMessageFilterCriteria((SipSession) session) ;
            criteria.restrictMethod("INVITE") ;
            criteria.restrictType(SipMessageFilterCriteria.RESPONSE) ;

            SipUserAgent.getInstance().addIncomingMessageListener(criteria, this) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Determine if the connection is unheld.  If so return FALSE and
     * continuing polling, otherwise return TRUE.
     */
    public boolean poll()
    {
        boolean bUnheld = false ;

        if (m_call != null)
        {
            synchronized (m_call)
            {
                TerminalConnection tc = m_call.getTerminalConnection(m_address.getAddress()) ;
                if ((tc != null) &&
                        (tc.getState() == CallControlTerminalConnection.TALKING))
                {
                    bUnheld = true ;
                }
            }
        }
        return bUnheld ;
    }


    /**
     * Clean up any resource for the poller.  In this implementation, we may
     * need to remove our incoming message listener from the Sip User Agent.
     */
    public void cleanup()
    {
        if (m_session != null)
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
