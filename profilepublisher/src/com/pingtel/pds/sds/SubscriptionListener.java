/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/SubscriptionListener.java#6 $
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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.URLPinger;
import com.pingtel.pds.sds.cache.SessionCache;
import com.pingtel.pds.sds.cache.SessionCacheException;
import com.pingtel.pds.sds.cmd.Command;
import com.pingtel.pds.sds.cmd.PhoneInvoker;
import com.pingtel.pds.sds.cmd.UpdateXpressaProfileCommand;
import com.pingtel.pds.sds.cmd.XpressaReceiver;
import com.pingtel.pds.sds.event.ISubscriptionListener;
import com.pingtel.pds.sds.event.SubscriptionEvent;
import com.pingtel.pds.sds.sip.SipProfileInfo;
import com.pingtel.pds.sds.sip.SipSessionContext;
import org.apache.log4j.Category;

/**
 * Title:        SunscriptionListener
 * Description:  This class handles the Callbacks from SIP
 * Copyright:    Copyright (c) 2001
 * Company:      Pingtel Corp.
 *
 * @author John P. Coffey
 * @version 1.0
 */
public class SubscriptionListener implements ISubscriptionListener {

    private static Category m_logger;

    static {
        m_logger = Category.getInstance( "pds" );
    }
    /**
     * This is a workaround for the class loader associated with a SIP callback thread
     * the C++ thread does not have an explicit ClassLoader associated with it. As a result
     * when it attempts to call up to JNDI to access the Sequence Numbers for the phone
     * it fails to load the InitialContext's factory classes.  Class.forName() is normally
     * how this works when there is not explicit class loader however this violates
     * the security rules as set by other VM arguments passed in.
     */
    static ClassLoader g_applicationClassLoader =
        Thread.currentThread().getContextClassLoader();

    /** Prevent cseq update from subscripton and enrollments, this object is
    the same as the one used by subscriptions */
    private Object m_cacheMutex = null;

    /** Database Helper utility object accesses the Subscription Database */
    private SessionCache m_sessionCache;

    /** This is only for the document root interface - depends on PoofileWriter */
    private ProfileListenerImpl m_profileListenerImpl;

    /** Constructor takes database cache helper to serialize to / from rdbms */
    public SubscriptionListener( SessionCache sessionCache,
                                 ProfileListenerImpl profileListenerImpl,
                                 Object cacheMutex ) {
        m_sessionCache = sessionCache;
        m_profileListenerImpl = profileListenerImpl;
        m_cacheMutex = cacheMutex;
    }

    /**
     * When a new SIP Subscribe message arrives from SIP we will get
     * a call back here from the underlying C++ layers.
     * <p>
     * The first thing we do is do is to remove any stale sessions
     * associated with this phone.  The To/From/Call-ID and Event Fields
     * are only consistent between successive renewals of the phone.  The
     * difficult case to solve is the situation where the phone looses
     * power and re-enrolls sessions with a different Call-ID and potentially
     * a different From Address (as the user configured a different identity
     * on their phone.
     * <p>
     * To remove the stale sessions, we must look up all the existing sessions
     * that are associated with the Mac Address of the phone.  Typically this
     * will yield 4 sessions for the new event model (or just one) for the
     * legacy 'sip-config' event type.  If however the phone lost power or the phone
     * was rebooted, we must ensure that any existing sessions are removed
     * if their session fields differ
     *
     * @param event  the callback event object contains a SubscriptionContext
     */
    public void subscriptionAdded( SubscriptionEvent event ) {
        // Log this message
        m_logger.debug( "subscription added!!" );

        synchronized ( m_cacheMutex ) {
            // See documentation for the g_mainClassLoader above, ensure
            // that the calling thread has the applicatino classLoader otherwise
            // the C++ sip stack will not be able to dynamically create a JNDI
            // naming context.
            if ( Thread.currentThread().getContextClassLoader() == null ) {
                Thread.currentThread().setContextClassLoader(g_applicationClassLoader);
            }

            // SDS must have document root for Content Delivery Server before proceeding
            if ( m_profileListenerImpl.getDocumentRoot() != null ) {
                // Get the sip session context from the incoming event
                SipSessionContext inputContext = event.getContext();
                try {
                    // find all contexts associated with the macAddress, note
                    // we look up all contexts associated with a particular Serial
                    // Number (MAC Address).  There may be several contexts assoc
                    // iated with a device as there may me many even types associated
                    // with a device.  This will be the case in the future when
                    // there will be a sepate subscribe per profile but that is
                    // not the case today.
                    HashMap deviceContexts =
                        m_sessionCache.
                            findSessionContexts(
                                inputContext.getMacAddress());

                    m_logger.debug ( "Found " + deviceContexts.size() +
                                    " session contexts for Serial# " +
                                    inputContext.getMacAddress() );

                    // Enroll the inputContext with the SDS database if this
                    // is the first time subscribe
                    if ( deviceContexts.isEmpty() ) {
                        // This is the first time so create the first subscription
                        // for this mac address
                        m_sessionCache.createSessionContext( inputContext );
                        // Retrieve all the sessions associated with this mac address
                        // the only time we will insert and query directly afterwards
                        // is at this creation time, it is a little more time consuming
                        // but we need a contextID for later processing below
                        deviceContexts = m_sessionCache.
                            findSessionContexts( inputContext.getMacAddress() );
                    }

                    // The 'Mac' Address already has sessions associated with it
                    // we are only interested in the ones that have a matching
                    // event-type as the 'To'/'From'/And 'Call-ID' fields MAY change.
                    // These may change if the Phone was Rebooted or if e.g. the
                    // user changed their credentials on the phone (the from
                    // address change).  When we find this one, we will send the
                    // NOTIFY message to the new address.  The key here is that the
                    // 'Mac' address matches.
                    Iterator iter = deviceContexts.entrySet().iterator();

                    while ( iter.hasNext() ) {
                        Map.Entry mapEntry = (Map.Entry) iter.next();
                        Integer contextID = (Integer) mapEntry.getKey();
                        SipSessionContext context = (SipSessionContext) mapEntry.getValue();

                        // there can be multiple device contexts, they are identified
                        // by the event type in the inputContext, so search for a match
                        // and then break out out (can only be one match)
                        if ( inputContext.getEvent().equals( context.getEvent() ) ) {
                            // is the phone un-subscribing (Expires header = 0)?
                            if ( inputContext.getExpirationSeconds() <= 0 ) {
                                m_logger.debug( "Unsubscribing Phone with 'Serial#/Event':" +
                                                inputContext.getMacAddress() + "/" +
                                                inputContext.getEvent() );
                                m_sessionCache.deleteSessionContext( contextID, false );
                            } else {
                                // Check to see if the input context is different from
                                // the restored database context, if it is update SDS DB
                                if ( !inputContext.equals( context ) ) {
                                    m_logger.debug("Subscription Context Changed");
                                    m_logger.debug("Input Context:\n\r" + inputContext.toString());
                                    m_logger.debug("Database Context:\n\r" + context.toString());

                                    // Update the database context with all of
                                    // the fields in the input context, note that
                                    // the input context will never have profileSequenceInfo
                                    // as it is not passed in with the subscribe message
                                    context.update( inputContext );
                                    m_logger.debug("Updating SDS Database Context");
                                    m_sessionCache.updateSessionContext( contextID, context );
                                }

                                // Bump the Output cseq field (the phone reads
                                // this to ensure that SIP message is not old)
                                m_sessionCache.updateSessionContextSequenceAndLease (
                                    contextID, new Integer (
                                    inputContext.getExpirationSeconds()) );

                                // for each matching device context, get the list of profiles
                                // it's requesting.  For the legacy code there will be 4 in a
                                // single context while with the newer subscribe mechanism will
                                // have a one context per profile
                                ArrayList requestedProfiles = context.getProfileSequenceInfo();

                                // Examine the URL's to see if they are reachable, if they are
                                // ensure that the sequence number is also valid (non -1)
                                if ( checkAndUpdatePingableProfiles ( context ) ) {
                                    // we changed the dbContext so persist the change to the DB
                                    // be very careful here as the context vs. input context
                                    // may have changed above, this would cause the context to
                                    // replace the inputContext (defeating the purpose), so persist
                                    // the updated input context instead
                                    m_sessionCache.updateSessionContext( contextID, context );
                                }

                                // Pingtel is the only vendor that supports Enrollments so far
                                if ( context.getVendor().indexOf( "Pingtel" ) >= 0 ) {
                                    m_logger.debug( "Sending Profile to Phone");

                                    // Fixed bug where I was using the restored DB context
                                    // fields in stead of teh input ones when a profile changed!
                                    Command command = new UpdateXpressaProfileCommand(
                                        XpressaReceiver.getInstance(),
                                        context.getMacAddress(),
                                        context.getToAddress(),
                                        context.getFromAddress(),
                                        context.getCallId(),
                                        context.getSequenceNumber(),
                                        m_profileListenerImpl.getDocumentRoot(),
                                        requestedProfiles );
                                    PhoneInvoker invoker = new PhoneInvoker(command);
                                    invoker.notifyPhone();
                                }
                            }
                            // found a matching context so break out of the while loop
                            break;
                        }
                    }
                } catch ( SessionCacheException ex ) {
                    m_logger.error("Problem reading/writing SusbcriptionCache", ex);
                    return;
                } catch ( PDSException ex ) {
                    m_logger.error( "Error: No document root received, " +
                                    "Unable to send profile to Phone", ex );
                    return;
                }
            } else {
                m_logger.error( "Error: No document root received, " +
                                "Unable to send profile to Phone");
            }
        }
    }

    /**
     * Unenrolling from the SDS Database
     *
     * @param event
     */
    public void subscriptionRemoved( SubscriptionEvent event ) {
        /** @todo: Implement ISubscriptionListener method */
        throw new UnsupportedOperationException("Method subscriptionRemoved() not yet implemented.");
    }

    /** Checks the profiles to see if they exist, if they do then the sequence number is checked */
    private boolean checkAndUpdatePingableProfiles( SipSessionContext context ) {
        boolean changed = false;
        ArrayList profiles = context.getProfileSequenceInfo();

        // @JC Profiles now default to no records in the DB rather than putting -1's in
        if ( profiles != null ) {
            // Profiles have been projected
            for (Iterator it = profiles.iterator(); it.hasNext(); ) {
                SipProfileInfo profileInfo = (SipProfileInfo)it.next();
                String url = m_profileListenerImpl.getDocumentRoot() + profileInfo.getURL();
                // Ping the URL so that we only send working URL's to the device
                if ( URLPinger.isValid(url) ) {
                    // if the profile sequence number is set to its default (-1)
                    // now is the time to see whether that profile exists
                    if ( profileInfo.getProfileSequenceNumber() < 0 ) {
                        // The sequence number can be (-1) for 2 reasons,
                        // (1) This is the first time phone has enrolled
                        // (2) the phone has enrolled but the Config server
                        // does not have a valid sequence number for it in which
                        // case we leave the sequence nmber to -1 and never send
                        // it to the phone
                        try {
                            profileInfo.setProfileSequenceNumber(
                                SequenceHelper.getInstance().getProfileSequenceNumber(
                                    profileInfo.getProfileType(),
                                    context.getMacAddress()));
                            // Success we got a real sequence from the config server
                            // flag this as a row change that needs to be persisted
                            // to the SDS database (this should be a positive number > 1)
                            changed = true;
                        } catch (PDSException ex) {
                            // hack no need to set the sequence number back to -1 as
                            // the sequence number is already -1 (this makes no change in
                            // the sequence number info so do not set the changed flag
                            m_logger.debug( "Error, Config Server has no sequence# for url: " + url +
                                            " Exception:" + ex.getMessage() );
                        }
                    }
                } else {
                    // mark the profile as unavailable with the -1 sequence number
                    profileInfo.setProfileSequenceNumber(-1);
                    // ensure that the change is detected
                    changed = true;
                    m_logger.debug( "Profile " + url + " unreachable" );
                }
            }
        }
        return changed;
    }
}