/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/startup/OutputStreamGobbler.java#2 $
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


public class OutputStreamGobbler extends OutputStream
{
    public void close() { }
                
    public void flush() { }

    public void write(byte[] b, int off, int len) { }

    public void write(int b) { }

    public void write(byte[] b) { }
}
