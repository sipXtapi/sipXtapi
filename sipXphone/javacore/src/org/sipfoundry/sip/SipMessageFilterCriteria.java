/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/SipMessageFilterCriteria.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
 
package org.sipfoundry.sip ;

/**
 * A SipMessageFilterCritera is used by with the SipUserAgent to restrict and
 * filter the SipMessages delievered to listeners added with through the
 * addIncomingMessageListener method.
 * <br><br>
 * Criteria can be limited by message type (REQUEST or RESPONSE), method type,
 * event type (only valid for methods SUBSCRIBE and NOTIFY), and sessions
 * (CallID, To URL, and From URL).
 * <br><br>
 * Constructors are provided for the different filtering criteria, however,
 * additional restrictions that combine different limitations can be added
 * using various restrict methods.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SipMessageFilterCriteria
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    private static final int _TYPE_MIN = -1 ;
    /** No message filtering (allow both Request and Responses) */
    public static final int NONE = -1 ;
    /** Request message filter type */
    public  static final int REQUEST = 0 ;
    /** Response message filter type */
    public  static final int RESPONSE = 1 ;
    private static final int _TYPE_MAX = 1 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private int             m_iMessageType ;     // Message type restriction
    private String          m_strMethod ;       // Method restriction
    private String          m_strEventType ;    // Event type restriction
    private SipSession      m_session ;         // Session Restriction


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs a criteria object that will filters SIP message by message
     * type.  Use the various mutator methods on this object to add additional
     * restrictions.
     *
     * @param messageType Defines filtering based on message type.
     */
    public SipMessageFilterCriteria(int iMessageType)
    {
        m_iMessageType = iMessageType ;

    }


    /**
     * Constructs a criteria object that will filters SIP message by session
     * identification.  Use the various restrict methods on this object to add
     * additional restrictions.
     *
     * @param session Defines filtering based on session identification (call
     *        id, to URL, from URL).
     */
    public SipMessageFilterCriteria(SipSession session)
    {
        m_session = session ;
        m_iMessageType = NONE ;
    }


    /**
     * Constructs a criteria object that will filters SIP message by method
     * type.  Use the various restrict methods on this object to add
     * additional restrictions.
     *
     * @param session Defines filtering based on method type.
     */
    public SipMessageFilterCriteria(String strMethod)
    {
        m_strMethod = strMethod ;
        m_iMessageType = NONE ;
    }



//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Adds a sip message type restriction to the filter criteria.
     *
     * @param messageType Defines filtering based on message type.
     */
    public void restrictType(int iMessageType)
    {
        m_iMessageType = iMessageType ;
    }


    /**
     * Gets the SipMessageType restriction for this filter criteria.
     *
     * @return Sip Message Type restriction or null if not set.
     */
    public int getTypeRestriction()
    {
        return m_iMessageType ;
    }


    /**
     * Adds a method type restriction to the filter criteria.
     *
     * @param messageType Defines filtering based on method type.
     */
    public void restrictMethod(String strMethod)
    {
        m_strMethod = strMethod ;
        boolean bMethodAllowed =
            SipUserAgent.getInstance().isMethodAllowed( strMethod);
        if( !bMethodAllowed ){
            throw new IllegalArgumentException
                    ("method "+ strMethod +" is not an allowed method by SIPStack.") ;
        }
        if (m_strMethod == null)
            m_strEventType = null ;
    }


    /**
     * Gets the method type restriction for this filter criteria.
     *
     * @return Method restriction or null if not set.
     */
    public String getMethodRestriction()
    {
        return m_strMethod ;
    }


    /**
     * Adds a event type restriction to the filter criteria.  Specifing an
     * EventType is only valid if a NOTIFY or SUBSCRIBE method is defined
     * prior.
     *
     * @param messageType Defines filtering based on event type.
     */
    public void restrictEventType(String strEventType)
        throws IllegalStateException
    {

        if (    (m_strMethod != null) &&
                m_strMethod.equalsIgnoreCase("NOTIFY")  ||
                m_strMethod.equalsIgnoreCase("SUBSCRIBE")) {
            m_strEventType = strEventType ;
        } else {
            throw new IllegalStateException("method must be NOTIFY or SUBSCRIBE to specify an event type") ;
        }
    }


    /**
     * Gets the event type restriction for this filter criteria.
     *
     * @return Event Type restriction or null if not set.
     */
    public String getEventTypeRestriction()
    {
        return m_strEventType ;
    }


    /**
     * Adds a session restriction to the filter criteria.
     *
     * @param messageType Defines filtering based on session (call id, to URL,
     *        from URL).
     */
    public void restrictSession(SipSession session)
    {
        m_session = session ;
    }


    /**
     * Gets the session restriction for this filter criteria.
     *
     * @return Session restriction or null if not set.
     */
    public SipSession getSessionRestriction()
    {
        return m_session ;
    }
}
