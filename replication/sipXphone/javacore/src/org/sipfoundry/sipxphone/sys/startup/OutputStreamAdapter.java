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


package org.sipfoundry.sipxphone.sys.startup ;

import java.io.* ;
import java.awt.* ;
import java.util.* ;


public abstract class OutputStreamAdapter extends OutputStream
{

    /**
     * prints an array of bytes, with the given length and offset.
     */
    public void write(byte[] bytes, int off, int len)
    {
        print( new String(bytes, off, len) );
    }


    public void write(int b)
    {
    }

    /**
     * prints an array of bytes..
     */
    public void write(byte[] b)
    {
        print( new String(b) );
    }


    /**
     * abstract method that needs to be implemented by subclasses to
     * tell how to print a String.
     */
    public abstract void print( String str );

}
