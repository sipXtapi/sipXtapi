/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.core;

/**
 * Implement this to add support for new devices to the system
 */
public interface Phone {

    /**
     * return this on getProfileSequenceNumber if you do not
     * support incrementing
     */
    public static final int SEQUENCE_NUMBER_NOT_SUPPORTED = -1;
    
    /**
     * Configuration context called shortly after instantation
     */
    public void setCoreDao(CoreDao dao);
    
    /**
     * File linking properties to this device
     * 
     * @return Absolute file path to xml file
     */
    public String getDefinitions();
        
    /**
     * Identifying string unique across all plugins 
     * 
     * @return e.g. cisco7960
     */
    public String getModelId();

    /**
     * How many files make-up the list of files this writer creates
     * 
     * @return zero or greater
     */
    public int getProfileCount();
    
    /**
     * Associate a profile w/a token to distinguish them as part of
     * SIP's SUBSCRIBE messages
     * 
     * @param profileIndex
     * @return e.g. "device-settings" or "user-settings"
     */
    public String getProfileSubscribeToken(int profileIndex);
    
    /**
     * What is the filename of the Nth profile
     *
     * TODO: Change this to take LogicalPhone
     * 
     * @param profileIndex
     * @param macAddress
     * 
     * @return No path, just filename
     */
    public String getProfileFileName(int profileIndex, String macAddress);
    
    /**
     * Where to send http message to notify phone there's a new profile
     * 
     * @param profileIndex
     * @return null if not supported
     */
    public String getProfileNotifyUrl(LogicalPhone logicalPhone, int profileIndex);
    
    /**
     * Increment a counter everytime there's a new profile for devices
     * can decide if they have the latest copy.
     * 
     * @param profileIndex
     * @return SEQUENCE_NUMBER_NOT_SUPPORTED if not supported
     */
    public int getProfileSequenceNumber(LogicalPhone logicalPhone, int profileIndex);
}
