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
import java.util.Collection;
import java.util.List;

import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.springframework.context.ApplicationContext;

public class ParkOrbitTestDb extends TestHelper.TestCaseDb {

    private CallGroupContext m_context;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (CallGroupContext) appContext.getBean(CallGroupContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsert("admin/callgroup/OrbitSeed.xml");
    }

    public void testLoadParkOrbit() throws Exception {
        ParkOrbit orbit = m_context.loadParkOrbit(new Integer(1001));
        assertEquals("sales", orbit.getName());
        assertTrue(orbit.isEnabled());
        assertEquals("sales", orbit.getName());
        assertEquals("401", orbit.getExtension());
        assertEquals("something.wav", orbit.getMusicOnHold());
    }
    
    public void testGetParkOrbits() throws Exception {
        Collection orbits = m_context.getParkOrbits();
        assertEquals(2, orbits.size());
        ParkOrbit orbit = (ParkOrbit) orbits.iterator().next();
        assertEquals("sales", orbit.getName());
        assertTrue(orbit.isEnabled());
        assertEquals("sales", orbit.getName());
        assertEquals("401", orbit.getExtension());
        assertEquals("something.wav", orbit.getMusicOnHold());
    }

    public void testStoreParkOrbit() throws Exception {
        ParkOrbit orbit = new ParkOrbit();
        orbit.setName("kuku");
        orbit.setDescription("kukuLine");
        orbit.setExtension("202");
        orbit.setEnabled(true);
        orbit.setMusicOnHold("tango.wav");
        m_context.storeParkOrbit(orbit);
        // table should be empty now
        ITable tableCallGroup = TestHelper.getConnection().createDataSet().getTable("park_orbit");
        assertEquals(3, tableCallGroup.getRowCount());
    }

    public void testRemoveParkOrbit() throws Exception {
        List ids = Arrays.asList(new Integer[] {
            new Integer(1001), new Integer(1002)
        });
        m_context.removeParkOrbits(ids);
        // table should be empty now
        ITable tableCallGroup = TestHelper.getConnection().createDataSet().getTable("park_orbit");
        assertEquals(0, tableCallGroup.getRowCount());
    }

    public void testClear() throws Exception {
        m_context.clear();
        // make sure the tables are empty
        ITable tableUserRing = TestHelper.getConnection().createDataSet().getTable("park_orbit");
        assertEquals(0, tableUserRing.getRowCount());
    }
    
    
    public void testGenerateAliases() throws Exception {
        List aliases = m_context.getAliases();
        assertNotNull(aliases);
        assertEquals(1, aliases.size());

        AliasMapping alias = (AliasMapping) aliases.get(0);
        System.err.println(alias);
        assertTrue(alias.getIdentity().startsWith("401"));
        assertTrue(alias.getContact().startsWith("401"));        
    }
}
