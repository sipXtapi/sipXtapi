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

import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.Group;

public class PhoneContextTestDb extends TestCase {

    private PhoneContext m_context;

    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
    }

    public void testClear() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        m_context.clear();

        TestHelper.cleanInsertFlat("phone/EndpointSeed.xml");
        m_context.clear();
    }
    
    public void testGetRootPhoneGroup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        Group root = m_context.loadRootGroup();
        assertNotNull(root);
        
        ITable actual = TestHelper.getConnection().createDataSet().getTable("group_storage");
        assertEquals(1, actual.getRowCount());
        assertEquals("phone", actual.getValue(0, "resource"));        
    }

    public void testCheckForDuplicateFieldsOnNew() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/EndpointSeed.xml");

        Phone p = m_context.newPhone("polycom600");
        PhoneData data = p.getPhoneData();
        data.setFactoryId("test");
        data.setSerialNumber("999123456");

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
        p.getPhoneData().setSerialNumber("000000000002");
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
        
        Map counts = m_context.getGroupMemberCountIndexedByGroupId();
        assertEquals(2, counts.size());
        assertEquals(new Integer(2), counts.get(new Integer(1001)));
        assertEquals(new Integer(1), counts.get(new Integer(1002)));
        assertNull(counts.get(new Integer(1003)));        
    }
}
