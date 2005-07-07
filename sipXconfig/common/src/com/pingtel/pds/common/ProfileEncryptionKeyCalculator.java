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

package com.pingtel.pds.common;


/**
 * ProfileEncryptionKeyCalculator is a helper class use to calculate the
 * digested value that is used as the key to encrypt Users profiles.
 *
 * @author butcheri
 */
public class ProfileEncryptionKeyCalculator {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////


//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public static String calculateProfileEncryptionKey(    String displayID,
                                                    String password) {
        return MD5Encoder.encode(displayID + ":" + password);
    }


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
