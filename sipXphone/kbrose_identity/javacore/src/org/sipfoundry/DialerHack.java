/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/DialerHack.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
package org.sipfoundry;

/**
 * This class is used in org.sipfoundry.sipxphone.service.test.TestAPI class
 * which in turn gets used by Gremlins test scripts. It has corresponding JNI
 * methods in org.sipfoundry_DialerHack.cpp class.
 */
public class DialerHack
{

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public static void main(String args[])
    {
        System.out.println("Hello World!!") ;

        int i = 0 ;

        while (true) {
            i++ ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    native public static void Dial(String strAddress) ;
    native public static void OffHook() ;
    native public static void OnHook() ;



}
