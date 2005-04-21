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
package org.sipfoundry.sipxconfig.admin.callgroup;

import java.util.Arrays;
import java.util.List;

import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class CallGroupContextImplTestDb extends TestHelper.TestCaseDb {

    private CallGroupContext m_context;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (CallGroupContext) appContext.getBean(CallGroupContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsert("admin/callgroup/dbdata/CallGroupSeed.xml");
    }

    public void testLoadCallGroup() throws Exception {
        CallGroup callGroup = m_context.loadCallGroup(new Integer(1001));
        assertEquals("sales",callGroup.getName());
        assertTrue(callGroup.isEnabled());
        assertEquals("sales",callGroup.getName());
        assertEquals("401",callGroup.getExtension());
        assertEquals("sales.sipx.1001",callGroup.getLineName());
    }

    public void testGetCallGroups() throws Exception {
        List callGroups = m_context.getCallGroups();
        assertEquals(2,callGroups.size());
        CallGroup callGroup = (CallGroup) callGroups.get(0);
        assertEquals("sales",callGroup.getName());
        assertTrue(callGroup.isEnabled());
        assertEquals("sales",callGroup.getName());
        assertEquals("401",callGroup.getExtension());
        assertEquals("sales.sipx.1001",callGroup.getLineName());
    }
    
    public void testLoadUserRing() throws Exception {
        CallGroup callGroup = m_context.loadCallGroup(new Integer(1002));
        List userRings = callGroup.getCalls();
        assertEquals(1,userRings.size());
        UserRing ring = (UserRing) userRings.get(0);
        assertSame(ring.getCallGroup(),callGroup);
        assertEquals(45,ring.getExpiration());
        assertEquals(AbstractRing.Type.DELAYED, ring.getType());
        assertEquals(0, ring.getPosition());
        assertEquals("testuser",ring.getUser().getDisplayId());
        
    }
    
    public void testStoreCallGroup() throws Exception {
        CallGroup group = new CallGroup();
        group.setName("kuku");
        group.setLineName("kukuLine");
        group.setExtension("202");
        group.setEnabled(true);
        m_context.storeCallGroup(group);
        // table should be empty now
        ITable tableCallGroup = TestHelper.getConnection().createDataSet().getTable("call_group");        
        assertEquals(3, tableCallGroup.getRowCount());                
    }

    public void testRemoveCallGroups() throws Exception {
        List ids = Arrays.asList(new Integer[] {
            new Integer(1001), new Integer(1002)
        });
        m_context.removeCallGroups(ids);
        // table should be empty now
        ITable tableCallGroup = TestHelper.getConnection().createDataSet().getTable("call_group");        
        assertEquals(0, tableCallGroup.getRowCount());        
        ITable tableUserRing = TestHelper.getConnection().createDataSet().getTable("user_ring");
        assertEquals(0, tableUserRing.getRowCount());                
    }
    
    public void testClear() throws Exception {
        m_context.clear();
        // make sure the tables are empty
        ITable tableCallGroup = TestHelper.getConnection().createDataSet().getTable("call_group");        
        assertEquals(0, tableCallGroup.getRowCount());                
        ITable tableUserRing = TestHelper.getConnection().createDataSet().getTable("user_ring");
        assertEquals(0, tableUserRing.getRowCount());                
    }
    
    public void testEditUserRing() throws Exception {
        final Integer testExpiration = new Integer(12);        
        
        CallGroup callGroup = m_context.loadCallGroup(new Integer(1002));
        List userRings = callGroup.getCalls();
        assertEquals(1,userRings.size());
        UserRing ring = (UserRing) userRings.get(0);
        ring.setExpiration(testExpiration.intValue());
        ring.setType(AbstractRing.Type.IMMEDIATE);
        m_context.storeCallGroup(callGroup);
        
        ITable tableUserRing = TestHelper.getConnection().createDataSet().getTable("user_ring");
        assertEquals(1, tableUserRing.getRowCount());
        assertEquals( testExpiration, tableUserRing.getValue(0,"expiration") );
        assertEquals( AbstractRing.Type.IMMEDIATE.getName(), tableUserRing.getValue(0,"ring_type") );
    }
}
