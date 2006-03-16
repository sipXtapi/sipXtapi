/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;

import junit.framework.TestCase;

import org.apache.log4j.Logger;
import org.apache.log4j.Priority;
import org.apache.log4j.SimpleLayout;
import org.apache.log4j.spi.LoggingEvent;

import sun.misc.Signal;

/**
 * Unclear if this works on windows
 */
public class SystemLogRotateAppenderTest extends TestCase {
    private SystemLogRotateAppender m_appender;
    private File m_file;
    
    protected void setUp() throws IOException {
        m_file = File.createTempFile(getClass().getName(), ".log");
        System.out.println(m_file.getPath());
        m_appender = new SystemLogRotateAppender(new SimpleLayout(), m_file.getPath(), false);        
    }
    
    public void testRotate() throws Exception {
        log("line 1");
        assertEquals(1, countLines());
        Signal.raise(new Signal("USR2"));
        Thread.sleep(100); // give time for signal to be delivered
        log("line 2");
        assertEquals(1, countLines());
    }
    
    public void testNoRotate() throws Exception {
        log("line 1");
        assertEquals(1, countLines());
        Thread.sleep(100); // give time for signal to be delivered
        log("line 2");
        assertEquals(2, countLines());
    }
            
    private void log(String msg) {
        Logger logger = Logger.getRootLogger();
        LoggingEvent event = new LoggingEvent("category name", logger, Priority.INFO, msg, null);
        m_appender.append(event);                
    }
    
    private int countLines() throws IOException {
        FileInputStream in = new FileInputStream(m_file);
        BufferedReader rdr = new BufferedReader(new InputStreamReader(in));
        int lines = 0;
        for (;rdr.readLine() != null; lines++);
        in.close();
        return lines;
    }
}
