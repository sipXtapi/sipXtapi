/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/util/SysLog.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.util;
import java.io.*;

/**
 * SysLog.java
 *
 * This class has utility methods to log messages to SysLog.
 * SysLog can be accesses using http://phoneaddress/cgi/syslog.cgi .
 *
 * Created: Fri Apr 12 11:22:25 2002
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */
public class SysLog
{
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * private constructor to guard against construction
     */
    private SysLog()
    {
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Logs the stack trace from Throwable t to SysLog.
     */
    public static void log(Throwable e)
    {
        e.printStackTrace();
        log( getStackTrace(e) ) ;
    }


    /**
     * Log a string to the syslog
     */
    public static void log(String string)
    {
        JNI_sysLogError(string, 0) ;
    }

    /**
     *Just prints out a debug message in the console.
     *Useful for debugging purpose. Make sure you comment
     *out or remove the debug call when done debugging.
     *@param obj the object from which debug mesage was printed.
     *@param message the message to be printed
     */
    public static void debug( Object obj, String message )
    {
        StringBuffer buffer = new StringBuffer();
        buffer.append("\n\n***DEBUG BEGIN***"+ new java.util.Date()+"*********\n");
        buffer.append(obj);
        buffer.append("\n");
        buffer.append(message);
        buffer.append("\n******************DEBUG END *************************\n\n");
        System.out.println(buffer.toString());
        buffer = null;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Converts a stack trace to a String.
     */
    private static String getStackTrace( Throwable e )
    {
        StringWriter sw = new StringWriter();
        PrintWriter out = new PrintWriter(sw);
        if( e != null )
        {
            e.printStackTrace(out);
        }
        return sw.getBuffer().toString();
    }


//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    private static native void JNI_sysLogError(String logMessage, int unused) ;
}
