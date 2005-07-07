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

public final class DeleteProfile extends ProfileWriterCommand {
    /**
     * 
     * @param profileType
     * @param macAddress
     * @param deviceURL
     * @param manufacturer
     * @param model
     * @param leafFileName
     */
    public DeleteProfile ( int profileType,
                           String macAddress,
                           String deviceURL,
                           String manufacturer,
                           String model,
                           String leafFileName ) {
        super( DELETE_PROFILE,
               profileType,
               macAddress,
               deviceURL,
               manufacturer,
               model,
               leafFileName);
    }
}
