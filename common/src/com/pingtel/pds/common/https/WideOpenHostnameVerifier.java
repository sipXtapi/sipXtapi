/*
 * $Id: //depot/OPENDEV/sipXconfig/common/src/com/pingtel/pds/common/https/WideOpenHostnameVerifier.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.common.https;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.SSLSession;


/**
 * WideOpenHostnameVerifier is required by SIPxchange in order to
 * be tolerant of server certificates where the name in the
 * certificate does not match the hostname/ip address.
 *
 * @author ibutcher
 * 
 */
public class WideOpenHostnameVerifier implements HostnameVerifier {

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
    public boolean verify(String s, SSLSession session) {
        return true;
    }

//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////    


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
