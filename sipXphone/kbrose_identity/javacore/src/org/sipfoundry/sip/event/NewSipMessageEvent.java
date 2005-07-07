/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/event/NewSipMessageEvent.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
 
package org.sipfoundry.sip.event ;

import org.sipfoundry.sip.* ;


/**
 * This Event is delievered as part of the NewSipMessageListener interface.
 * Subscribers can review the new SipMessage along with determining if this
 * message is a final response.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class NewSipMessageEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
//// 
    private SipMessage m_message ;          // New Sip Message
    private boolean    m_bFinalResponse ;   // Is this a final response?


//////////////////////////////////////////////////////////////////////////////
// Construction
//// 
    
    /**
     * Constructs a new event object with the required events parameters.
     *
     * @param message The SIP Message associated with this event
     * @param bFinalResponse true the associatd message is a final response.
     */
    public NewSipMessageEvent(SipMessage message, boolean bFinalResponse)
    {
        m_message = message ;
        m_bFinalResponse = bFinalResponse ;
    }
    

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get the Sip message associated with this event.
     *
     * @return The SipMessage associated with this event.
     */
    public SipMessage getMessage() 
    {
        return m_message ;
    }
    
    
    /**
     * Determine if the Sip message associated with this event is a final 
     * response.
     *
     * @return true if this is a final reponse, otherwise false.
     */
    public boolean isFinalResponse()
    {
        return m_bFinalResponse ;
    }
}
