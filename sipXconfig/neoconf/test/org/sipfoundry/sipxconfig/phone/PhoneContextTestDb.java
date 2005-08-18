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

import java.util.Map;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public class PhoneContextTestDb extends TestCase {

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
}
