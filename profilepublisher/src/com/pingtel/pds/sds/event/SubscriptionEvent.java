/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/event/SubscriptionEvent.java#6 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.sds.event ;

import com.pingtel.pds.sds.sip.SipSessionContext;


/**
 * SubscriptionEvent delievered as part of the ISubscriptionListener interface.
 *
 * @see ISubscriptionListener
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SubscriptionEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Object m_objSource ;            // Source entity firing off the event
    private SipSessionContext m_context ;   // subscription context


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Construct a SubscriptionEvent with the specified source object and
     * context.
     *
     * @param objSource The object responsible for firing this event
     * @param context The subscription context
     */
    public SubscriptionEvent(Object objSource, SipSessionContext context)
    {
        m_objSource = objSource ;
        m_context = context ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Get the source of this event.
     *
     * @return Object responsible for firing off this event
     */
    public Object getSource()
    {
        return m_objSource ;
    }


    /**
     * Get the subscription context
     *
     * @return SubscriptionContext of this event
     */
    public SipSessionContext getContext()
    {
        return m_context ;
    }
}