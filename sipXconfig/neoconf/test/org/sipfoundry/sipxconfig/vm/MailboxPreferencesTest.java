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
package org.sipfoundry.sipxconfig.vm;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;

public class MailboxPreferencesTest extends TestCase {
    
    private XmlReaderImpl<MailboxPreferences> m_reader;
    private XmlWriterImpl<MailboxPreferences> m_writer;
    
    protected void setUp() {
        m_reader = new MailboxPreferencesReader();
        m_writer = new MailboxPreferencesWriter();
        m_writer.setVelocityEngine(TestHelper.getVelocityEngine());
    }
    
    public void testReadPreferences() {
        InputStream in = getClass().getResourceAsStream("200/mailboxprefs.xml");
        MailboxPreferences prefs = m_reader.readObject(new InputStreamReader(in));
        IOUtils.closeQuietly(in);      
        assertEquals("outofoffice", prefs.getActiveGreeting());
        assertEquals("dhubler@pingtel.com", prefs.getEmailAddress());
        assertTrue(prefs.isAttachVoicemailToEmail());
    }
    
    public void testWritePreferences() throws IOException {
        StringWriter actual = new StringWriter();
        MailboxPreferences prefs = new MailboxPreferences();
        prefs.setEmailAddress("dhubler@pingtel.com");
        prefs.setActiveGreeting("outofoffice");
        m_writer.writeObject(prefs, actual);
        StringWriter expected = new StringWriter();
        InputStream expectedIn = getClass().getResourceAsStream("expected-mailboxprefs.xml");
        IOUtils.copy(expectedIn, expected);
        IOUtils.closeQuietly(expectedIn);
        assertEquals(expected.toString(), actual.toString());
    }

    public void testReadInitialPreferences() {
        InputStream in = getClass().getResourceAsStream("initial-mailboxprefs.xml");
        m_reader.readObject(new InputStreamReader(in));        
        IOUtils.closeQuietly(in);              
    }
}
