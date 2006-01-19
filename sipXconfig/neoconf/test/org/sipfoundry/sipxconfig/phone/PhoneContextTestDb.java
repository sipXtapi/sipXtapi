/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Map;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public class PhoneContextTestDb extends SipxDatabaseTestCase {

    private PhoneContext m_context;

    private SettingDao m_settingContext;

    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);

        m_settingContext = (SettingDao) TestHelper.getApplicationContext().getBean(
                SettingDao.CONTEXT_NAME);
    }

    public void testClear() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        m_context.clear();

        TestHelper.cleanInsertFlat("phone/EndpointSeed.xml");
        m_context.clear();
    }

    public void testCheckForDuplicateFieldsOnNew() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/EndpointSeed.xml");

        Phone p = m_context.newPhone(Phone.MODEL);
        p.setSerialNumber("999123456");

        try {
            m_context.storePhone(p);
            fail("should have thrown Duplicate*Exception");
        } catch (UserException e) {
            // ok
        }
    }

    public void testCheckForDuplicateFieldsOnSave() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/DuplicateSerialNumberSeed.xml");

        Phone p = m_context.loadPhone(new Integer(1000));
        p.setSerialNumber("000000000002");
        try {
            m_context.storePhone(p);
            fail("should have thrown DuplicateFieldException");
        } catch (UserException e) {
            // ok
        }
    }

    public void testGetGroupMemberCountIndexedByGroupId() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/GroupMemberCountSeed.xml");

        Map counts = m_settingContext.getGroupMemberCountIndexedByGroupId(Phone.class);
        assertEquals(2, counts.size());
        assertEquals(new Integer(2), counts.get(new Integer(1001)));
        assertEquals(new Integer(1), counts.get(new Integer(1002)));
        assertNull(counts.get(new Integer(1003)));
    }

    /**
     * this test is really for PhoneTableModel in web context
     */
    public void testGetPhonesByPageSortedByModel() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/SamplePhoneSeed.xml");

        Collection page1 = m_context.loadPhonesByPage(null, 0, 4, "modelId", true);
        Phone[] phones = (Phone[]) page1.toArray(new Phone[page1.size()]);
        assertEquals("00001", phones[0].getSerialNumber());
        assertEquals("00003", phones[1].getSerialNumber());
        assertEquals("aa00004", phones[2].getSerialNumber());
        assertEquals("00002", phones[3].getSerialNumber());
    }
    
    public void testLoadPhones() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/SamplePhoneSeed.xml");

        Collection page1 = m_context.loadPhones();
        assertEquals(4, page1.size());
        Phone[] phones = (Phone[]) page1.toArray(new Phone[page1.size()]);
        assertEquals("00001", phones[0].getSerialNumber());
        assertEquals("00002", phones[1].getSerialNumber());
        assertEquals("00003", phones[2].getSerialNumber());
        assertEquals("aa00004", phones[3].getSerialNumber());
    }

    public void testGetAllPhoneIds() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/SamplePhoneSeed.xml");

        Collection result = m_context.getAllPhoneIds();
        assertEquals(4, result.size());
        Integer[] ids = (Integer[]) result.toArray(new Integer[result.size()]);
        for (int i = 0; i < ids.length; i++) {
            assertEquals(new Integer(1000 + i), ids[i]);
        }
    }

    public void testGetPhoneIdBySerialNumber() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/SamplePhoneSeed.xml");
        assertEquals(new Integer(1002), m_context.getPhoneIdBySerialNumber("00003"));
        assertEquals(new Integer(1003), m_context.getPhoneIdBySerialNumber("Aa:00004"));
        assertEquals(null, m_context.getPhoneIdBySerialNumber("won't find this guy"));
    }

    public void testCountPhones() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/SamplePhoneSeed.xml");
        assertEquals(4, m_context.getPhonesCount());
    }

    public void testGetGroupByName() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/SamplePhoneSeed.xml");
        
        Group g1 = m_context.getGroupByName("phone group 1", false);
        assertNotNull(g1);
        assertEquals("phone group 1", g1.getName());
        
        Group g2 = m_context.getGroupByName("bongo", false);
        assertNull(g2);
        assertEquals(2, getConnection().getRowCount("group_storage"));

        g2 = m_context.getGroupByName("bongo", true);
        assertNotNull(g2);
        assertEquals("bongo", g2.getName());
        
        assertEquals(3, getConnection().getRowCount("group_storage"));
    }    
    
    public void testCountPhonesInGroup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/SamplePhoneSeed.xml");
        assertEquals(1, m_context.getPhonesInGroupCount(new Integer(1001)));
        assertEquals(2, m_context.getPhonesInGroupCount(new Integer(1002)));
    }

    public void testAddToGroup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/GroupMemberCountSeed.xml");

        assertEquals(0, TestHelper.getConnection().getRowCount("phone_group",
                "where phone_id = 1001 AND group_id = 1002"));

        m_context.addToGroup(new Integer(1002), Collections.singleton(new Integer(1001)));

        assertEquals(1, TestHelper.getConnection().getRowCount("phone_group",
                "where phone_id = 1001 AND group_id = 1002"));
    }

    public void testRemoveFromGroup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/GroupMemberCountSeed.xml");

        Integer[] ids = {
            new Integer(1001), new Integer(1002)
        };

        assertEquals(2, TestHelper.getConnection().getRowCount("phone_group",
                "where phone_group.group_id = 1001"));

        m_context.removeFromGroup(new Integer(1001), Arrays.asList(ids));

        assertEquals(0, TestHelper.getConnection().getRowCount("phone_group",
                "where phone_group.group_id = 1001"));
    }
}
