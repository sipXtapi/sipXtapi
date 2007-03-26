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
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import java.io.IOException;
import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class LgNortelPhonebookTest extends TestCase {
    private ProfileGenerator m_pg;
    private MemoryProfileLocation m_location;

    protected void setUp() {
        m_location = new MemoryProfileLocation();
        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        pg.setProfileLocation(m_location);
        m_pg = pg;
    }
    
    public void testPhonebook() throws IOException {
        IMocksControl phonebookEntryControl = EasyMock.createNiceControl();
        PhonebookEntry phonebookEntry = phonebookEntryControl.createMock(PhonebookEntry.class);
        phonebookEntry.getFirstName();        
        phonebookEntryControl.andReturn("Joe");
        phonebookEntry.getNumber();
        phonebookEntryControl.andReturn("1234");
        phonebookEntryControl.replay();

        LgNortelPhonebook book = new LgNortelPhonebook(Collections.singleton(phonebookEntry));

        m_pg.generate(book, "phonebook");
        List<String> list = IOUtils.readLines(m_location.getReader());
        assertEquals(2, list.size());
        assertEquals("1, \"Joe \", 1234,", list.get(1));

        phonebookEntryControl.verify();
    }

}
