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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.sipfoundry.sipxconfig.vm.Voicemail.MessageDescriptor;

public class VoicemailTest extends TestCase {

    public void testReadMessageDescriptor() throws IOException {
        InputStream in = getClass().getResourceAsStream("200/inbox/00000001-00.xml");
        MessageDescriptor md = Voicemail.readMessageDescriptor(in);
        assertNotNull(md);
        assertEquals("Voice Message 00000002", md.getSubject());
    }
    
    public void testGetDate() throws ParseException {
        DateFormat formatter = new SimpleDateFormat(MessageDescriptor.TIMESTAMP_FORMAT);
        Date date = formatter.parse("Tue, 9-Jan-2007 02:33:00 PM EST");
        Calendar cal = Calendar.getInstance(TimeZone.getTimeZone("EST"), Locale.ENGLISH);
        cal.setTime(date);
        cal.toString();
        assertEquals(14, cal.get(Calendar.HOUR_OF_DAY));
    }
    
    /**
     * XCF-1519
     */
    public void testGetDateWithNoTimezone() throws ParseException {
        DateFormat formatter = new SimpleDateFormat(MessageDescriptor.TIMESTAMP_FORMAT_NO_ZONE);
        formatter.setLenient(true);
        formatter.parse("Sun, 11-Feb-2007 04:07:31 AM");        
    }
    
    public void testLoadDescriptor() {
        File mailstore = new File(TestUtil.getTestSourceDirectory(getClass()));
        Voicemail vm = new Voicemail(mailstore, "200", "inbox", "00000001-00");
        assertEquals("Voice Message 00000002", vm.getSubject());        
        assertEquals("\"Douglas+Hubler\"<sip:201@nuthatch.pingtel.com>;tag%3D53585A61-338ED896", vm.getFrom());        
    }
    
    public void testRealbasename() {
        Voicemail vm = new Voicemail(new File("."), "200", "inbox", "00000001-00");
        assertEquals("00000001", vm.getRealBasename());        
    }
    
    public void testMove() throws IOException {
        File mailstore = createTestMailStore();
        Voicemail vmOrig = new Voicemail(mailstore, "200", "inbox", "00000001-00");
        vmOrig.move("deleted");
        Voicemail vmNew = new Voicemail(mailstore, "200", "deleted", "00000001-00");
        assertEquals("Voice Message 00000002", vmNew.getSubject());
        // nice, not critical
        FileUtils.deleteDirectory(mailstore);
    }
    
    static File createTestMailStore() throws IOException {
        File mailstore = new File(TestUtil.getTestSourceDirectory(VoicemailTest.class));
        File testMailstore = new File(TestHelper.getTestDirectory() + '/' + System.currentTimeMillis());
        testMailstore.mkdirs();
        FileUtils.copyDirectory(new File(mailstore, "200"), new File(testMailstore, "200"));        
        return testMailstore;
    }
    
    public void testDelete() throws IOException {
        File mailstore = createTestMailStore();
        MailboxManagerImpl mgr = new MailboxManagerImpl();
        mgr.setMailstoreDirectory(mailstore.getAbsolutePath());
        List<Voicemail> vm = mgr.getVoicemail("200", "inbox"); 
        assertEquals(1, vm.size());

        vm.get(0).delete();

        assertEquals(0, mgr.getVoicemail("200", "inbox").size());

        // nice, not critical
        FileUtils.deleteDirectory(mailstore);        
    }
    
    public void testWriteDescriptor() throws IOException {
        ByteArrayOutputStream original = new ByteArrayOutputStream();
        InputStream in = getClass().getResourceAsStream("200/inbox/00000001-00.xml");
        IOUtils.copy(in, original);
        InputStream inMemory = new ByteArrayInputStream(original.toByteArray());
        MessageDescriptor md = Voicemail.readMessageDescriptor(inMemory);
        md.setSubject("testWriteDescriptor");
        ByteArrayOutputStream actual = new ByteArrayOutputStream();
        Voicemail.writeMessageDescriptor(md, actual);
        
        ByteArrayOutputStream expected = new ByteArrayOutputStream();
        InputStream inExpected = getClass().getResourceAsStream("write-expected.xml");
        IOUtils.copy(inExpected, expected);
        
        assertEquals(new String(expected.toByteArray()), new String(actual.toByteArray()));
    }
    
    public void testSave() throws IOException {
        File mailstore = createTestMailStore();
        Voicemail vm = new Voicemail(mailstore, "200", "inbox", "00000001-00");
        assertEquals("Voice Message 00000002", vm.getSubject());
        vm.setSubject("new subject");
        vm.save();
        
        Voicemail saved = new Voicemail(mailstore, "200", "inbox", "00000001-00");
        assertEquals("new subject", saved.getSubject());
    }
}
