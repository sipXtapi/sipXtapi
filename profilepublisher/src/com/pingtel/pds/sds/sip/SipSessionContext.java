/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.sds.sip;

import java.io.PrintStream;
import java.util.*;

import com.pingtel.pds.sds.sip.SipMultiValuedHeader;
import com.pingtel.pds.common.PDSDefinitions;

import org.apache.log4j.Category;

/**
 * Session context.  This is really more of a data object, however, it
 * acts like a session/context.
 *
 * @author Robert J. Andreasen, Jr., John P. Coffey
 */
public class SipSessionContext {

    private static Category m_logger;

    static {
        m_logger = Category.getInstance( "pds" );
    }

    /** These represent the Header=Value SIP Headers */
    private Properties  m_singleValuedSipHeaders;

    /** These represent the Header=Value;n1=v1;n2=v2...nn=vn SIP Headers */
    private HashMap m_multiValuedSipHeaders;

    /**
     * the entries are profile (key) and the sequence num (Integer) as represented
     * by the profile_sequence_nums table in the database.
     */
    private ArrayList m_profileSequenceInfo;

    /**
     * each time the context is used to generate messages it must track
     * the sequence number, This will eventually disappear when the new
     * API arrives and the SIP stack will handle this for us
     */
    private int m_outputCSeq;

    /** returns the single valued headers like expires adn cseq */
    public Properties getSingleValuedSipHeaders() {
        return m_singleValuedSipHeaders;
    }

    /** returns the multi valued headers that can accept url parameters */
    public HashMap getMultiValuedSipHeaders() {
        return m_multiValuedSipHeaders;
    }

    /**
     * Added to satisfy the ability to reconstruct a SessionContext
     * from a database row.  I do not like the idea of making this a public
     * constructor for anyone to construct as it depends on the DB
     */
    public SipSessionContext( Properties singleValuedSipHeaders,
                              HashMap multiValuedSipHeaders,
                              ArrayList profileSequenceInfo,
                              int outputCSeq) {
        m_singleValuedSipHeaders = singleValuedSipHeaders;
        m_multiValuedSipHeaders = multiValuedSipHeaders;
        m_profileSequenceInfo = profileSequenceInfo;
        m_outputCSeq = outputCSeq;
    }

    /**
     * Construct a Session context from the input SipMessage
     *
     * @param The source subcription request
     *
     * @exception IllegalArgumentException if the passed SipMessage is null or
     *            not a valid 'Config-Event' Session request.
     */
    public SipSessionContext( SipMessage request ) {
        m_multiValuedSipHeaders = new HashMap ();

        // Parse the 'From' SIP Header
        SipMultiValuedHeader fromHeader =
            new SipMultiValuedHeader(
                request.getHeaderFieldValue(
                    SipMessage.SIP_FROM_ADDRESS));

        m_multiValuedSipHeaders.put(
            SipMessage.SIP_FROM_ADDRESS, fromHeader);

        // Parse the 'To' SIP Header
        SipMultiValuedHeader toHeader =
            new SipMultiValuedHeader(
                request.getHeaderFieldValue(
                    SipMessage.SIP_TO_ADDRESS));

        m_multiValuedSipHeaders.put(
            SipMessage.SIP_TO_ADDRESS, toHeader);

        // Parse the 'call-Id' SIP Header
        SipMultiValuedHeader callIDHeader =
            new SipMultiValuedHeader(
                request.getHeaderFieldValue(
                    SipMessage.SIP_CALL_ID));

        m_multiValuedSipHeaders.put(
            SipMessage.SIP_CALL_ID, callIDHeader);

        // Parse the 'Event' SIP Header
        SipMultiValuedHeader eventHeader =
            new SipMultiValuedHeader(
                request.getHeaderFieldValue(
                    SipMessage.SIP_EVENT));

        m_multiValuedSipHeaders.put(
            SipMessage.SIP_EVENT, eventHeader);

        m_singleValuedSipHeaders = new Properties();

        // Old legacy problem where the phone's SUBSCRIBE sends an underscore
        // instead of a '-' for the Config strings these fields will only be
        // encountered if the eventHeader is 'sip-config' and the phone software
        // release is prior to 1.1 (thanks to a protocol fix from Serbhi
        if ( eventHeader.getPrimaryValue().equalsIgnoreCase(SipMessage.SIP_LEGACY_SUBSCRIBE_EVENT) ) {
            String protocols =
                request.getHeaderFieldValue(
                    SipMessage.SIP_SUBSCRIBE_ALLOWED_PROTOCOLS );

            m_singleValuedSipHeaders.setProperty(
                SipMessage.SIP_SUBSCRIBE_ALLOWED_PROTOCOLS, protocols );

            String profiles =
                request.getHeaderFieldValue(
                    SipMessage.SIP_SUBSCRIBE_REQUIRED_PROFILES );

            m_singleValuedSipHeaders.setProperty(
                SipMessage.SIP_SUBSCRIBE_REQUIRED_PROFILES, profiles );
        }

        // Some versions of the phone do not send in an Expires Header
        // if it is not provided we default one to 60 days to that the
        // houseKeeper cache thread does not expire it
        String expirationSecs =
            request.getHeaderFieldValue(
                SipMessage.SIP_SUBSCRIBE_EXPIRATION_SECS );

        // this is actually a number in seconds
        m_singleValuedSipHeaders.setProperty(
            SipMessage.SIP_SUBSCRIBE_EXPIRATION_SECS,
            expirationSecs );

        // query the incoming Sequence number, this is distinct from
        // the outgoing one which must be tracked separately
        String inputCSeq =
            request.getHeaderFieldValue(
                SipMessage.SIP_CSEQ );

        // this is actually a number in seconds
        m_singleValuedSipHeaders.setProperty(
            SipMessage.SIP_CSEQ, inputCSeq);

        // Default the next outgoing sequence number to this value even though
        // there is an input one (ignore it for now)
        m_outputCSeq = 1000;

        // creating the SessionContext from a sip message (rather than the DB)
        m_profileSequenceInfo = null;
    }

    /**
     * This method returns a list of required profiles that the Sessino is requesting
     * The old model used to require 4 Profiles in one Session however the new model
     * should only have one Required Profile for each request in which case the event
     * type is the profile.
     *
     * @return array list of SipProfileInfo objects
     */
    public ArrayList getProfileSequenceInfo () {
        return m_profileSequenceInfo;
    }

    /**
     *  Setter for the profile sequence info
     */
    public void setProfileSequenceInfo (final ArrayList profileSequenceInfo ) {
      m_profileSequenceInfo = profileSequenceInfo;
    }

    /**
     * Get the client vendor ID.
     *
     * @return String containing client's vendor ID or null if not available.
     */
    public String getVendor() {
        SipMultiValuedHeader fromHeaders =
            (SipMultiValuedHeader)m_multiValuedSipHeaders.
                get( SipMessage.SIP_FROM_ADDRESS );
        return fromHeaders.getProperty(SipMessage.SIP_SUBSCRIBE_VENDOR);
    }

    /**
     * Get the client model ID.
     *
     * @return String containing client's model ID or null if not available.
     */
    public String getMacAddress() {
        SipMultiValuedHeader fromHeaders =
            (SipMultiValuedHeader)m_multiValuedSipHeaders.
                get( SipMessage.SIP_FROM_ADDRESS );
        return fromHeaders.getProperty(SipMessage.SIP_SUBSCRIBE_MACADDRESS);
    }

    /**
     * Get the client model ID.
     *
     * @return String containing client's model ID or null if not available.
     */
    public String getModel() {
        SipMultiValuedHeader fromHeaders =
            (SipMultiValuedHeader)m_multiValuedSipHeaders.
                get( SipMessage.SIP_FROM_ADDRESS );
        return fromHeaders.getProperty(SipMessage.SIP_SUBSCRIBE_MODEL);
    }

    /**
     * Get the client version ID.
     *
     * @return String containing client's version ID or null if not available.
     */
    public String getVersion() {
        SipMultiValuedHeader fromHeaders =
            (SipMultiValuedHeader)m_multiValuedSipHeaders.
                get( SipMessage.SIP_FROM_ADDRESS );
        return fromHeaders.getProperty(SipMessage.SIP_SUBSCRIBE_VERSION);
    }

    /**
     * Get client serial number.
     *
     * @return String containing client's serial number or null if not available.
     */
    public String getSerialNumber() {
        SipMultiValuedHeader fromHeaders =
            (SipMultiValuedHeader)m_multiValuedSipHeaders.
                get( SipMessage.SIP_FROM_ADDRESS );
        return fromHeaders.getProperty(SipMessage.SIP_SUBSCRIBE_SERIALNUMBER);
    }

    /**
     * returns the number of seconds that this lease has to live
     *
     * @return
     */
    public int getExpirationSeconds() {
        String str = m_singleValuedSipHeaders.getProperty(
            SipMessage.SIP_SUBSCRIBE_EXPIRATION_SECS );
        return Integer.parseInt(str);
    }

    /**
     * Returns the complete 'Call-Id' SIP field, including the field and other parameters
     * @return stringified version of the from address including tag values etc.
     */
    public String getCallId() {
        return m_multiValuedSipHeaders.get( SipMessage.SIP_CALL_ID).toString();
    }

    /**
     * Returns the complete 'From' SIP field, including the field and other parameters
     * @return All the values after the primary 'From' field value.
     */
    public String getFromAddress() {
        return m_multiValuedSipHeaders.get( SipMessage.SIP_FROM_ADDRESS).toString();
    }

    /**
     * Returns the complete 'To' SIP field, including the field and other parameters
     * @return All the values after the primary 'To' field value.
     */
    public String getToAddress() {
        return m_multiValuedSipHeaders.get( SipMessage.SIP_TO_ADDRESS).toString();
    }

    /**
     * Returns the complete 'Event' SIP field, including the field and other parameters
     * @return All the values after the primary 'Event' field value.
     */
    public String getEvent() {
        return m_multiValuedSipHeaders.get( SipMessage.SIP_EVENT).toString();
    }

    /**
     * returns the next output sequence number, this is distinct from the CSeq that
     * accompanies the subscribe message as these are 2 completely different
     * sequence counters associated with subscribe and notify messages respectively
     * @return
     */
    public int getSequenceNumber () {
        return m_outputCSeq;
    }

    /**
     *
     * @param sequenceNumber
     */
    public void setSequenceNumber (int sequenceNumber) {
        m_outputCSeq = sequenceNumber;
    }

    /**
     * @param strExpiration
     * @return
     */
    private int getExpirationField(String strExpiration) {
        int expirationInSecs = -1;

        if (strExpiration != null) {
            strExpiration = strExpiration.trim();
            try {
                expirationInSecs = Integer.parseInt(strExpiration);
            } catch (NumberFormatException ex) {
                m_logger.error( "Error while trying to parse expiration " +
                                    strExpiration,
                                ex );
            }
        }
        return expirationInSecs;
    }

    /**
     * Print the context to the designated writer object
     *
     * @param writer
     */
    public void dump(PrintStream writer) {
        writer.println();
        writer.println("Dumping SessionContext: ");
        writer.println();
        writer.println( this.toString() );
    }

    /**
     * To String method should display the SessionContext fields
     */
    public String toString() {
        StringBuffer sb = new StringBuffer();
        Enumeration singleNames = m_singleValuedSipHeaders.propertyNames();
        sb.append ( "SINGLES: \n\r");
        while ( singleNames.hasMoreElements() ) {
            String name = (String) singleNames.nextElement();
            sb.append ( name + " : " + m_singleValuedSipHeaders.get( name ) + "\n\r" );
        }
        sb.append ( "MULTIS: \n\r");
        for ( Iterator i = m_multiValuedSipHeaders.entrySet().iterator(); i.hasNext(); ) {
            Map.Entry entry = (Map.Entry) i.next();
            sb.append ( entry.getKey() + " : " + entry.getValue() + "\n\r");
        }


//        if ( m_multiValuedSipHeaders.get( SipMessage.SIP_TO_ADDRESS) != null ) {
//            sb.append( "           To: " +
//                    m_multiValuedSipHeaders.get( SipMessage.SIP_TO_ADDRESS).toString() + "\n\r" );
//        }
//        if ( m_multiValuedSipHeaders.get( SipMessage.SIP_FROM_ADDRESS) != null ) {
//            sb.append( "         From: " +
//                    m_multiValuedSipHeaders.get( SipMessage.SIP_FROM_ADDRESS).toString() + "\n\r" );
//        }
//        if ( m_multiValuedSipHeaders.get( SipMessage.SIP_CALL_ID) != null ) {
//            sb.append( "      Call-Id: " +
//                    m_multiValuedSipHeaders.get( SipMessage.SIP_CALL_ID).toString() + "\n\r" );
//        }
//        if ( m_multiValuedSipHeaders.get( SipMessage.SIP_EVENT) != null ) {
//            sb.append( "        Event: " +
//                    m_multiValuedSipHeaders.get( SipMessage.SIP_EVENT).toString() + "\n\r" );
//        }
        return sb.toString();
    }

    /**
     * Creates a notify message
     *
     * @param strBody
     * @return
     */
    public SipMessage createNotifyMessage(String strBody) {
        SipMessage notify = new SipMessage();

        notify.setRequestHeader("NOTIFY", getFromAddress(), "SIP/2.0");
        notify.setHeaderFieldValue( SipMessage.SIP_TO_ADDRESS,
                                    getFromAddress());
        notify.setHeaderFieldValue( SipMessage.SIP_FROM_ADDRESS,
                                    getToAddress());
        notify.setHeaderFieldValue( SipMessage.SIP_CALL_ID,
                                    getCallId());
        notify.setHeaderFieldValue( SipMessage.SIP_CSEQ,
                                    m_outputCSeq + " NOTIFY");
        notify.setHeaderFieldValue( SipMessage.SIP_EVENT,
                                    SipMessage.SIP_LEGACY_SUBSCRIBE_EVENT);
        notify.setBody(strBody);

        m_logger.debug("-----------------------------");
        m_logger.debug("Notify Message Content:" );
        m_logger.debug("-----------------------------" );
        m_logger.debug( notify.toString() );
        m_logger.debug("-----------------------------" );

        return notify;
    }

    /**
     * Update the Context with newer fields from the SIP SUBSCRIBE message
     * we must be very careful here not to overwrite the profileSequence
     * info as this is not present in the newContext
     */
    public void update( final SipSessionContext newContext ) {
        this.m_singleValuedSipHeaders = newContext.getSingleValuedSipHeaders();
        this.m_multiValuedSipHeaders = newContext.getMultiValuedSipHeaders();
    }

    /**
     * Returns the sip session to the user
     */
    public SipSession getSipSession () {
        return new SipSession(getFromAddress(), getToAddress(), getCallId(), getEvent());
    }

    /**
     * Compare all the attributes of the new request to the incoming one.
     * The CSequence and the expires field should be ignored in this compare
     * as the objective here is to ensure that the incoming one is the same db ref
     *
     * @param arg Session context to compare against
     * @return true of these are identical, false otherwise
     */
    public boolean equals (Object arg) {
        if ( arg instanceof SipSessionContext ) {
            if (arg == this)
                return true;
            else {
                // I am not including the getSingleValuedSipHeaders in this
                // comparison since these values can be dynamic between invocatinos
                SipSessionContext rhs = (SipSessionContext)arg;
                return ( m_multiValuedSipHeaders.equals(
                            rhs.getMultiValuedSipHeaders()) /* &&
                         m_singleValuedSipHeaders.equals(
                            rhs.getSingleValuedSipHeaders()) */);
            }
        }
        return false;
    }
}