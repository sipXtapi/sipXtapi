/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/startup/ConsoleOutputStreamAdapter.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys.startup ;


/**
 * OutputStream to be used to print String to console.
 */
public class ConsoleOutputStreamAdapter extends OutputStreamAdapter
{
    /**
     * abstract method from superclass that is implemented to print a
     * string to console.
     */
    public void print( String str){
        PingerApp.JNI_consolePrint(str, 0) ;
    }
}
