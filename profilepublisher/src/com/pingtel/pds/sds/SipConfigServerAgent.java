/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/SipConfigServerAgent.java#6 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.sds;

import java.util.Map;
import java.util.ArrayList;
import java.util.Iterator;
import java.io.IOException;

import org.apache.log4j.Category;

/** @todo  these will go when the C++ layer calls back appropriate SipConfigServerAgent */
import com.pingtel.pds.sds.sip.SipProfileInfo;
import com.pingtel.pds.sds.sip.SipMessage;
import com.pingtel.pds.sds.sip.SipSessionContext;

import com.pingtel.pds.sds.event.SubscriptionEvent;
import com.pingtel.pds.sds.event.ISubscriptionListener;


/**
 * Java Bindings for the SipConfigServerAgent.  The PDS/SDS can add listeners
 * for notification of enrollment and unenrollment.  Also, as notifies must be
 * be directed via this agent.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SipConfigServerAgent
{

    private static Category m_logger = Category.getInstance( "pds" );

    /** Singleton agent instance */
    private  static SipConfigServerAgent m_instance;

    /** Subscription Listeners - Should only be one */
    private ArrayList m_vSubscriptionListeners;

    /**
     * Private constuctor - singleton
     */
    private SipConfigServerAgent() throws IOException {
        // Load the SIP Stack DLL
        initializeUserAgent();
        m_vSubscriptionListeners = new ArrayList();
    }

    /**
     * Get a singleton instance to the SipConfigServerAgent
     */

    // IB - we should move the DLL load code to a static block at the top of the
    // class and make m_instance get initialized in the field declaration.   If
    // we do this then we can remove the synchronized modifier.
    synchronized public static SipConfigServerAgent getInstance() throws IOException {
        // @JC Ensure that multiple threads do not try
        // to load the DLL at once
        if (m_instance == null) {
            m_instance = new SipConfigServerAgent();
            // Load the DLL
            JNI_initializeCallback();
        }
        return m_instance;
    }


    /**
     * Initialize the SIP User Agent, loads the native C++ DLL and initializes
     * it under the covers
     */
    private static void initializeUserAgent() throws IOException {
        m_logger.debug("-> initializing User Agent...");

        // Load the C++ DLL SIP Stack under covers
        try {
            m_logger.debug( "attempting to load sds library" );
            System.loadLibrary("publish-sipagent");
            m_logger.debug( "sucessfully loaded sds library" );
        } catch (UnsatisfiedLinkError ex) {
            m_logger.fatal( "unable to sds load native library", ex );
            throw new IOException (ex.getMessage());
        } catch (SecurityException ex) {
            m_logger.fatal("unable to load native library", ex);
            throw new IOException (ex.getMessage());
        }
        JNI_initializeUserAgent();
    }

    /**
     * Add an subscription listener to this agent.  Listeners are notified
     * whenever a client subscribes to or unsubscribes from this agent.
     *
     * @param listener Encrollment listener that will receive enrollment
     *        events
     */
    public void addSubscriptionListener( ISubscriptionListener listener ) {
        synchronized (m_vSubscriptionListeners) {
            if (!m_vSubscriptionListeners.contains(listener) ) {
                m_vSubscriptionListeners.add(listener);
            }
        }
    }

    /**
     * Removes an subscription listener to this agent.  Listeners will no
     * longer receive notifitions when clients subscribe to or unsubscribe
     * from this agent.
     *
     * @param listener Encrollment listener that will stop receiving
     *        enrollment events
     */
    public void removeSubscriptionListener(ISubscriptionListener listener)
    {
        synchronized (m_vSubscriptionListeners) {
            m_vSubscriptionListeners.remove(listener);
        }
    }

    /**
     * Send a notify message containing the profile data in the designated
     * notifyData object to the subscribe identified by the supplied
     * Session context.
     *
     * @param context the SessionContext supplied as part of the
     *        enrollment process.
     * @param sipProfileData containing an ordered list of hashmap's each containing
     *        a key and value pair.
     *
     * @exception java.lang.IllegalArgumentException if any of the parameters are null or
     *            if the data objects within the profileData parameter are not
     *            of type ProfileData.
     */
    public void sendUpdate( SipSessionContext context,
                            ArrayList profileData )
        throws IOException
    {
        // Validate Parameters
        if ( profileData == null ) {
            throw new IllegalArgumentException();
        }

        // Generate body for the Sip Message
        StringBuffer body = new StringBuffer();
        try {
            for (Iterator it = profileData.iterator(); it.hasNext(); ) {
                Map profileEntry = (Map) it.next();
                for (Iterator keys = profileEntry.keySet().iterator(); keys.hasNext();) {
                    String strKey = (String) keys.next();
                    SipProfileInfo data = (SipProfileInfo) profileEntry.get(strKey);
                    body.append(strKey + ": " + data.toString() + "\r\n");
                }
            }
        } catch (ClassCastException ex) {
            m_logger.error( "error in sendUpdate: " + ex.toString(), ex );
            throw new IllegalArgumentException(ex.toString());
        }
        // Do some magic and actually send it!
        SipMessage msg = context.createNotifyMessage(body.toString());
        sendMessage(msg);
    }


    /** Send a SIP Message to the SIP Stack */
    public void sendMessage(SipMessage message) throws IOException {
        String sipOutputString = message.toString();

        m_logger.debug("---------------------------------------------------");
        m_logger.debug("-> Sending SIP Message...");
        m_logger.debug( sipOutputString );
        m_logger.debug("---------------------------------------------------");


        // @todo ensure that the signature of this interface method
        // throws IOException right now it does'nt
        long t1 = new java.util.Date().getTime();
        JNI_sendSipMessage( sipOutputString );
        long t2 = new java.util.Date().getTime();
        m_logger.info("Time to send message through JNI_sendSipMessage: " + ( t2-t1 ));
    }

    protected void fireSubscriptionAdded( SipSessionContext context )  {
        m_logger.debug( "-> Firing Subscription Added Event" );

        // Fire off callback events to all of our listeners
        synchronized (m_vSubscriptionListeners) {
            for ( Iterator iter = m_vSubscriptionListeners.iterator();
                  iter.hasNext();) {

                ISubscriptionListener listener = (ISubscriptionListener) iter.next();
                if (listener != null) {
                    try {
                        listener.subscriptionAdded( new SubscriptionEvent (this, context) );
                    } catch (Exception ex) {
                        m_logger.error( "unhandled exception caught from listener", ex );
                    }
                }
            }
        }
    }


    /**
     * This method is invoked from JNI as a result to an enrollment callback.
     *
     * DO NOT CHANGE THIS METHOD SIGNATURE without updating the JNI binds
     */
    private static void addSubscription( String strMessageContent ) {
        m_logger.debug( "-> Subscription Added " + strMessageContent);

        SipMessage message = new SipMessage(strMessageContent);
        SipSessionContext context = new SipSessionContext(message);
        /** @todo add the IOException signatire to the above method */
        try {
            getInstance().fireSubscriptionAdded( context );
        } catch (IOException ex) {
            m_logger.error( "error adding subscription" + strMessageContent, ex );
        }
    }

    /**
     * Initialize this class as a callback.  This allows SUBSCRIPTION methods
     * of type sip-config to bubble out of the "addSubscription" method.
     */
    private static native void JNI_initializeCallback();


    /**
     * Initialize the SipConfigServerAgent.  This method starts up the
     * components needed for the Sip User Agent
     */
    private static native void JNI_initializeUserAgent();


    /**
     * Sends a generic SIP message out through the user agent
     * @todo ensure that this method throws an appropriate IOException
     */
    private static native void JNI_sendSipMessage(String strMessage);

}
