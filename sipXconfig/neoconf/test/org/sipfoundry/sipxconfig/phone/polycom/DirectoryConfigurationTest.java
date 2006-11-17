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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.CharArrayReader;
import java.io.CharArrayWriter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.Collection;
import java.util.Collections;

import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.polycom.DirectoryConfiguration.PolycomPhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class DirectoryConfigurationTest extends XMLTestCase {
    PolycomPhone m_phone;
    Phonebook m_phonebook;
    PhoneTestDriver m_tester;
    
    protected void setUp() {
        XMLUnit.setIgnoreWhitespace(true);
        m_phone = new PolycomPhone();        
    }
    
    public void testTransformRows() throws Exception {
        IMocksControl phonebookEntryControl = EasyMock.createControl();
        PhonebookEntry phonebookEntry = phonebookEntryControl.createMock(PhonebookEntry.class);
        phonebookEntry.getFirstName();
        phonebookEntryControl.andReturn("Dora");
        phonebookEntryControl.replay();
        
        DirectoryConfiguration dir = new DirectoryConfiguration(null, null);
        Collection<PolycomPhonebookEntry> collection = dir.transformRows(Collections.singleton(phonebookEntry));
        assertEquals("Dora", collection.iterator().next().getFirstName());
        
        phonebookEntryControl.verify();
    }
    
    public void testGenerateDirectory() throws Exception {
        
        IMocksControl phonebookEntryControl = EasyMock.createControl();
        PhonebookEntry phonebookEntry = phonebookEntryControl.createMock(PhonebookEntry.class);
        phonebookEntry.getFirstName();
        phonebookEntryControl.andReturn("Dora");
        phonebookEntry.getLastName();
        phonebookEntryControl.andReturn("Explorer");
        phonebookEntry.getNumber();
        phonebookEntryControl.andReturn("210");
        phonebookEntryControl.replay();

        Collection<PhonebookEntry> entries = Collections.singleton(phonebookEntry);
        DirectoryConfiguration dir = new DirectoryConfiguration(m_phone, entries);
        dir.setVelocityEngine(TestHelper.getVelocityEngine());
        
        CharArrayWriter out = new CharArrayWriter();
        dir.generateProfile(m_phone.getDirectoryTemplate(), out);       
        
        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-directory.xml");
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);            
        Reader generatedXml = new CharArrayReader(out.toCharArray());

        // helpful debug
        // System.out.println(new String(out.toCharArray()));

        // also helpful
        // Writer w = new FileWriter("/tmp/delme");
        // IOUtils.write(out.toCharArray(), w);
        // w.close();

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();        

        phonebookEntryControl.verify();
    }
}
