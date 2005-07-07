/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

 
package org.sipfoundry.sipxphone.sys.util ;

public class NetUtilities
{
//////////////////////////////////////////////////////////////////////////////
// Public static methods
////
    public static boolean checkIPAddress(String strIPAddress)
    {
        return JNI_checkIPAddress(0, strIPAddress) ;
    }
    
    public static boolean checkNetmask(String strNetmask)
    {        
        return JNI_checkNetmask(0, strNetmask) ;
    }
    
    public static boolean isSameNetwork(String strDestIPAddress,
                                        String strIPAddress,
                                        String strNetMask)
    {
        return JNI_isSameNetwork(0, strDestIPAddress, strIPAddress, strNetMask) ;
    }
    
//////////////////////////////////////////////////////////////////////////////
// Native Methods
////    
    protected static native boolean JNI_checkIPAddress(long lHandle, String strIPAddress) ;
    protected static native boolean JNI_checkNetmask(long lHandle, String strNetmask) ;
    protected static native boolean JNI_isSameNetwork(long lHandle, String strDestIPAddress, String strIPAddress, String strNetMask) ;            
}
