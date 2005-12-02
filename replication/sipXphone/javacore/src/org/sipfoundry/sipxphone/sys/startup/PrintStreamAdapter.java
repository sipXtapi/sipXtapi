/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/startup/PrintStreamAdapter.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.sys.startup;
import java.io.*;
/**
 * PrintStreamAdapter.java
 *
 * PrintStream used to redirect messages to console/syslog
 * depending on the OutputStreamAdapter used.
 *
 * Created: Tue Apr 23 13:37:40 2002
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class PrintStreamAdapter extends PrintStream{

    private OutputStreamAdapter m_outputStreamAdapter;

    /**
     * Constructor.
     */
    public PrintStreamAdapter(OutputStreamAdapter outputStreamAdapter){
        super(outputStreamAdapter);
        m_outputStreamAdapter = outputStreamAdapter;
    }

    /**
     * prints a String with a new line.
     */
    public void println(String s){
        StringBuffer strBuffer = new StringBuffer(s);
        strBuffer.append("\n");
        m_outputStreamAdapter.print(strBuffer.toString());
    }

    /**
     * prints a String.
     */
    public void print(String s){
        m_outputStreamAdapter.print(s);
    }

    /**
     * prints an array of characters with a new line.
     */
    public void println(char x[]){
        println( new String(x) ) ;
    }



}// PrintStreamAdapter
