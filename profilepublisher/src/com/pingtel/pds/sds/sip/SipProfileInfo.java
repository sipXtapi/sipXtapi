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

/**
 * This class is used to bind the url (actually offset from base folder)
 * with the sequence number for a profile.
 */
public class SipProfileInfo {
    private String m_url;           // The URL to our data
    private int m_profileType;      // the type of profile (install device etc)
    private int m_iSequenceNumber;  // Sequence Number of the data

    /**
     * Constructor requiring both the profile's URL and sequence number
     *
     * @exception IllegalArgumentException thrown if url is null or an
     *            empty string.
     */
    public SipProfileInfo( int profileType,
                           String url,
                           int iSequenceNumber ) {
        if ( (url == null) || ( url.length() == 0 ) ) {
            throw new IllegalArgumentException("URL must not be null or blank") ;
        }
        m_profileType = profileType;
        m_url = url;
        m_iSequenceNumber = iSequenceNumber ;
    }

    /**
     * Get the profile's data source URL
     */
    public int getProfileType() {
        return m_profileType;
    }

    /**
     * Get the profile's data source URL
     */
    public String getURL() {
        return m_url;
    }

    /** Setter for the sequence number */
    public void setProfileSequenceNumber (int iSequenceNumber) {
        m_iSequenceNumber = iSequenceNumber;
    }

    /**
     * Get the profile's data source sequence number
     */
    public int getProfileSequenceNumber() {
        return m_iSequenceNumber;
    }


    /**
     * Get the string representation of this profile data object
     * this method is used as part of body generation
     */
    public String toString() {
        return "url=" + m_url + ", sequence=" + Integer.toString( m_iSequenceNumber ) ;
    }
}
