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

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.test.TestUtil;
import org.sipfoundry.sipxconfig.vm.Voicemail.MessageDescriptor;

public class VoicemailTest extends TestCase {

    public void testReadMessageDescriptor() throws IOException {
        InputStream in = getClass().getResourceAsStream("200/inbox/00000001-00.xml");
        Voicemail mv = new Voicemail(new File("."), "creeper", "inbox", "00000001-00");
        MessageDescriptor md = mv.readMessageDescriptor(in);
        assertNotNull(md);
    }
    
    public void testGetDate() {
        Date date = MessageDescriptor.parseTimestamp("Tue, 9-Jan-2007 02:33:00 PM EST");
        Calendar cal = Calendar.getInstance(TimeZone.getTimeZone("EST"), Locale.ENGLISH);
        cal.setTime(date);
        cal.toString();
        assertEquals(14, cal.get(Calendar.HOUR_OF_DAY));
    }
    
    public void testLoadDescriptor() {
        File mailstore = new File(TestUtil.getTestSourceDirectory(getClass()));
        Voicemail vm = new Voicemail(mailstore, "200", "inbox", "00000001-00");
        assertEquals("Voice Message 00000002", vm.getSubject());        
    }
}
