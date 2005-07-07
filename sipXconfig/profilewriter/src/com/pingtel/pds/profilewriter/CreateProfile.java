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

package com.pingtel.pds.profilewriter;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2001</p>
 * <p>Company: </p>
 * @author unascribed
 * @version 1.0
 */

public final class CreateProfile extends ProfileWriterCommand {
    // this is the raw data stream that needs to be persisted
    private byte[] m_profileData;

    private int m_sequenceNumber;

    /**
     * 
     * @param profileType
     * @param macAddress
     * @param deviceURL
     * @param manufacturer
     * @param model
     * @param leafFileName
     * @param profileData
     * @param sequenceNumber
     */
    public CreateProfile ( int profileType,
                           String macAddress,
                           String deviceURL,
                           String manufacturer,
                           String model,
                           String leafFileName,
                           byte[] profileData,
                           int sequenceNumber ) {
        super( CREATE_PROFILE,
               profileType,
               macAddress,
               deviceURL,
               manufacturer,
               model,
               leafFileName);
        // @JC May need to clone this but try without first
        m_profileData = profileData;
        m_sequenceNumber = sequenceNumber;
    }

    public int getSequenceNumber() {
        return m_sequenceNumber;
    }

    public byte[] getProfileData() {
        return m_profileData;
    }
}
