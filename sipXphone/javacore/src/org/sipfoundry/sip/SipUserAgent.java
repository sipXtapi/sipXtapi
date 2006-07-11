/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/SipUserAgent.java#2 $
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

import org.sipfoundry.sip.event.* ;
import java.io.IOException ;
import org.sipfoundry.util.CountingSemaphore ;


/**
 * The SipUserAgent exposes the Pingtel SIP Stack to Java developers.
 * Developers can send both synchronous and asynchronous message along
 * receiving filtered notifications of inbound messages.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SipUserAgent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private static SipUserAgent m_instance ;   // Singleton instance of the UA


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Private constructor guards multiple instantiation.  Use the
     * getInstance() method to obtain an reference to the SipUserAgent.
     */
    private SipUserAgent() { }


    /**
     * Get a singleton instance the the SipUserAgeent
     */
    public static SipUserAgent getInstance()
    {
        if (m_instance == null) {
            m_instance = new SipUserAgent() ;

        }
        return m_instance ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Adds a supported extension to the Pingtel SIP Stack.  By default, a sip
     * message that requires an unknown extension (through the "require"
     * header field) is automatically sent a 420 (Bad Extension) response.
     * This method allows you to define additional extension so that
     * applications can review and/or handle these requests.
     *
     * @param strExtension The extension name that the SIP Stack should
     *        accept.
     */
    public void addAllowExtension(String strExtension)
    {
        // Trim parameter if not null
        if (strExtension != null)
            strExtension = strExtension.trim() ;

        // Validate before adding
        if ((strExtension != null) && (strExtension.length() > 0)) {
            JNI_addExtension(strExtension, null) ;
        } else {
            throw new IllegalArgumentException("invalid extension") ;
        }
    }

    /**
     * Creates a new subscribe message automatically enrolled into
     * refresh manager. Make sure expires is set correctly!<br/>
     * NOTE: Ideally SipMessage in java and C++ have a one to one
     * relationship so this trading one SipMessage for another one would
     * not be nec.
     *
     * @return a new SipMessage with some fields added by the refresh manager
     */
    public SipMessage newSubscribeMessage(SipMessage msg)
    {
        String modified = JNI_newSubscribeMessage(msg.toString());
        return (modified != null ? new SipMessage(modified) : null);
    }

    /**
     * Adds a supported method to the Pingtel SIP Stack.
     * This method allows you to define additional methods so that
     * applications can review and/or handle these requests.
     *
     * @param strMethod The method name that the SIP Stack should
     *        accept.
     */
    public void addAllowMethod(String strMethod)
    {
        // Trim parameter if not null
        if (strMethod != null)
            strMethod = strMethod.trim() ;

        // Validate before adding
        if ((strMethod != null) && (strMethod.length() > 0)) {
            JNI_addMethod(strMethod, null) ;
        } else {
            throw new IllegalArgumentException("invalid method") ;
        }
    }

    /**
     * Is the method supported by the Pingtel SIP Stack?
     * @param strMethod The method name you are checking to see if the
     * SIP Stack accepts it.
     */
    public boolean isMethodAllowed( String strMethod ){
        boolean bRet = false;
        if (strMethod != null)
            strMethod = strMethod.trim() ;
        // Validate before adding
        if ((strMethod != null) && (strMethod.length() > 0)) {
            bRet = JNI_isMethodAllowed(strMethod, null);
        }
        return bRet;
    }


    /**
     * Posts a SIP Message to a remote User Agent (UA).  This method simply
     * hands off the SIP Message and does not wait (or block) for a response.
     *
     * @param message The SipMessage to send.
     *
     * @exception SipMessageFormatException thrown if the SipMessage is
     *            formatted incorrectly.
     * @exception IOException thrown if an error occures while attempting
     *            to transmit the message.
     * @exception IllegalArgumentException thrown if the specified message
     *            is null.
     */
    public void postMessage(SipMessage message)
        throws SipMessageFormatException, IOException, IllegalArgumentException
    {
        // Validate Parameters
        if (message == null)
            throw new IllegalArgumentException("sip message cannot be null") ;

        JNI_postRequest(message.toString(), null) ;
    }


    /**
     * Posts a SIP Message to a remote User Agent (UA).  This method simply
     * hands off the SIP Message and does not wait (or block) for a response.
     * All responses, including provisional and non-final responses, are
     * delievered to the specified NewSipMessageListener.
     *
     * @param request The SIP request to send.
     * @param listener The NewSipMessageListener that will be notified of all
     *        provisional, non-final, and final responses.
     *
     * @exception SipMessageFormatException thrown if the SipMessage is
     *            formatted incorrectly.
     * @exception SipTimeoutException thrown if no response is received
     *            to the response within a timely manor.
     * @exception IOException thrown if an error occures while attempting
     *            to transmit the message.
     * @exception IllegalArgumentException thrown if the specified request
     *            is null.
     */
    public void postRequest(SipRequest request, NewSipMessageListener listener)
        throws SipMessageFormatException, IOException, SipTimeoutException,
               IllegalArgumentException
    {
        if (listener == null)
            postMessage(request) ;
        else {
            // Validate Parameters
            if (request == null)
                throw new IllegalArgumentException("sip message cannot be null") ;

            JNI_postRequest(request.toString(), listener) ;
        }
    }


    /**
     * Sends a SIP message to a remote User Agent (UA).  This method will wait
     * (block) until a response is received.  If the message cannot be sent or
     * a timeout occurs while waiting for a response, a fake response is
     * returned with the appropriate response code.
     *
     * @param request The SIP request to send.
     *
     * @return The SipResponse received as a result of the designated request.
     *
     * @exception SipMessageFormatException thrown if the SipMessage is
     *            formatted incorrectly.
     * @exception SipTimeoutException thrown if no response is received
     *            to the response within a timely manor.
     * @exception IOException thrown if an error occures while attempting
     *            to transmit the message.
     * @exception IllegalArgumentException thrown if the specified request
     *            is null.
     */
    public SipResponse sendRequest(SipRequest request)
        throws SipMessageFormatException, SipTimeoutException, IOException,
               IllegalArgumentException
    {
        // Validate Parameters
        if (request == null)
            throw new IllegalArgumentException("sip message cannot be null") ;

        SipResponse response = null ;

        // Post the request with semaphore/response listener object.  This
        // will unblock this thread as soon as a final response is received.
        icFinalResponseSemaphore semaphore = new icFinalResponseSemaphore() ;
        JNI_postRequest(request.toString(), semaphore) ;

        if (semaphore.down()) {
            response = (SipResponse) semaphore.getFinalResponse() ;
        }

        return response ;
    }


    /**
     * Adds a NewSipMessageListener with the specified message filter
     * criteria. The listener will be delievered SIP messages that match the
     * specified filter criteria.
     *
     * @param criteria Defines the filter criteria used to limit message
     *        delievered to the specified NewSipMessageListener.
     * @param listener The NewSipMessageListener that will be notified of all
     *        SIP message that meet the specified criteria.
     *
     * @exception IllegalArgumentException thrown if the specified listener
     *            or filter criteria is null.
     */
    public int addIncomingMessageListener(SipMessageFilterCriteria criteria,
                                           NewSipMessageListener    listener)
        throws IllegalArgumentException
    {
        // Validate Parameters
        if (criteria == null)
            throw new IllegalArgumentException("criteria cannot be null") ;
        if (listener == null)
            throw new IllegalArgumentException("cannot add null listener") ;

        // Flatten out the criteria object to ease the JNI efforts
        int iType = -1 ;
        String strMethod = null ;
        String strEventType = null ;
        String strCallId = null ;
        String strRemoteURL = null ;
        String strLocalURL = null ;

        // Init Type Restriction
        iType = criteria.getTypeRestriction() ;

        // Init Method Restriction
        strMethod = criteria.getMethodRestriction() ;

        // Init EventType Restriction
        strEventType = criteria.getEventTypeRestriction() ;

        // Init Session Restriction
        SipSession session = criteria.getSessionRestriction() ;
        if (session != null) {
            strCallId = session.getCallID() ;
            strRemoteURL = session.getRemoteURL() ;
            strLocalURL = session.getLocalURL() ;
        }

/*
        System.out.println("Adding Sip Incoming Message Listener: ") ;
        System.out.println("     iType: " + iType) ;
        System.out.println("    Method: " + strMethod) ;
        System.out.println(" EventType: " + strEventType) ;
        System.out.println("   Call Id: " + strCallId) ;
        System.out.println("    Remote URL: " + strRemoteURL) ;
        System.out.println("  Local URL: " + strLocalURL) ;
*/

        // Finally, make the call
        JNI_addIncomingListener(iType, strMethod, strEventType, strCallId,
                strRemoteURL, strLocalURL, listener, listener.hashCode()) ;

	//Comment by Raghu:- The return value is just a dummy as of now.  
	// Bob's changelist #41860 changed the implementation of this method 
	// to return an int. Since the xpresstest framework uses this method, 
	// I need the two branches to have similar signature for xpresstest to 
	// work on both branches
	return 0 ; 
    }


    /**
     * Removes all listeners added for the specified listener object.  This
     * listener will no longer receive notification of new messages.
     *
     * @param listener The NewSipMessageListener that will not longer be
     *        notified of SIP message.
     *
     * @exception IllegalArgumentException thrown if the specified listener
     *            is null.
     */
    public void removeIncomingMessageListener(NewSipMessageListener listener)
        throws IllegalArgumentException
    {
        // Validate Parameters
        if (listener == null)
            throw new IllegalArgumentException("cannot remove null listener") ;

        JNI_removeIncomingListener(listener, listener.hashCode()) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////


//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////

    private class icFinalResponseSemaphore extends CountingSemaphore
        implements NewSipMessageListener
    {
        SipMessage m_finalResponse ;        // The final response message


        /**
         * Constructor, initializing to an empty semaphore
         */
        public icFinalResponseSemaphore()
        {
            super(0, true) ;
        }


        /**
         * If we receive a final response as part of the NewSipMessageListener
         * interface, then up() and unblock whoever is waiting.
         */
        public void newMessage(NewSipMessageEvent event)
        {
            if (event.isFinalResponse()) {
                m_finalResponse = event.getMessage() ;
                up() ;
            }
        }

        /**
         * Return the final response for any interested party
         */
        public SipMessage getFinalResponse()
        {
            return m_finalResponse ;
        }

    }

//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////
    private static native String JNI_newSubscribeMessage(String strSipMessage) ;

    private static native void JNI_postRequest(String strSipMessage,
            NewSipMessageListener listener) ;

    private static native void JNI_addIncomingListener(int iType,
            String strMethod, String strEventType, String CallId,
            String strRemoteURL, String strLocalURL,
            NewSipMessageListener listener, long lHashCode) ;

    private static native void JNI_removeIncomingListener(
            NewSipMessageListener listener, long lHashCode) ;

    private static native void JNI_addExtension(String strExtension, String strExtension2) ;
    private static native void JNI_addMethod(String strMethod, String strMethod2) ;
    private static native boolean JNI_isMethodAllowed(String strMethod, String strMethod2) ;
    private static native boolean JNI_isExtensionAllowed(String strExtension, String strExtension2) ;

}
