/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;

public class RegistrationContextImplTest extends TestCase {

    private RegistrationContextImpl m_builder;

    protected void setUp() throws Exception {
        m_builder = new RegistrationContextImpl();
    }

    public void testGetRegistrations() throws Exception {
        InputStream is = getClass().getResourceAsStream("registration.test.xml");
        List registrations = m_builder.getRegistrations(is);
        assertEquals(2, registrations.size());
        for (int i = 0; i < 2; i++) {
            RegistrationItem ri = (RegistrationItem) registrations.get(i);
            assertEquals(2000 + i, ri.getExpires());
            assertTrue(ri.getUri().startsWith("300" + i));
            assertTrue(ri.getContact().indexOf("Doe") > 0);
        }
    }

    public void testGetRegistrationsEmpty() throws Exception {
        String xml = "<items/>";
        InputStream stream = new ByteArrayInputStream(xml.getBytes());
        List registrations = m_builder.getRegistrations(stream);
        assertTrue(registrations.isEmpty());
    }

    public void testGetUrl() throws Exception {
        m_builder.setServer("localhost");
        m_builder.setPort("8888");

        assertEquals("https://localhost:8888/sipdb/registration.xml", m_builder.getUrl());
    }

    public void testCleanRegistrations() {
        String[][] regData = {
            {
                "contact1", "10"
            }, {
                "contact2", "11"
            }, {
                "contact1", "12"
            }, {
                "contact3", "13"
            }, {
                "contact2", "9"
            }, {
                "contact2", "11"
            }
        };
        List regs = new ArrayList();
        for (int i = 0; i < regData.length; i++) {
            RegistrationItem item = new RegistrationItem();
            item.setContact(regData[i][0]);
            item.setExpires(Long.parseLong(regData[i][1]));
            regs.add(item);
        }

        List cleanRegs = m_builder.cleanRegistrations(regs);
        assertEquals(3, cleanRegs.size());
        assertEquals("contact1", ((RegistrationItem) cleanRegs.get(0)).getContact());
        assertEquals("contact2", ((RegistrationItem) cleanRegs.get(1)).getContact());
        assertEquals("contact3", ((RegistrationItem) cleanRegs.get(2)).getContact());
        assertEquals(12, ((RegistrationItem) cleanRegs.get(0)).getExpires());
        assertEquals(11, ((RegistrationItem) cleanRegs.get(1)).getExpires());
        assertEquals(13, ((RegistrationItem) cleanRegs.get(2)).getExpires());
    }
}
