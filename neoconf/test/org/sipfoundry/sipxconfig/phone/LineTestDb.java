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

import java.util.Collection;

import junit.framework.TestCase;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.User;

/**
 * You need to call 'ant reset-db-patch' which clears a lot of data in your
 * database. before calling running this test. 
 */
public class LineTestDb extends TestCase {

    private PhoneContext m_context;
    
    private CoreContext m_core;

    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        m_core = (CoreContext) TestHelper.getApplicationContext().getBean(
                CoreContext.CONTEXT_BEAN_NAME);
    }

    public void testAddingLine() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/dbdata/AddLineSeed.xml");

        Phone phone = m_context.loadPhone(new Integer(1));
        assertEquals(2, phone.getLines().size());
        User user = m_core.loadUserByDisplayId("testuser");

        LineData thirdLine = new LineData();
        thirdLine.setUser(user);
        thirdLine.setFolder(m_context.loadRootLineFolder());
        phone.addLine(phone.createLine(thirdLine));
        m_context.storePhone(phone);

        // reload data to get updated ids
        m_context.flush();
        Phone reloadedPhone = m_context.loadPhone(new Integer(1));
        Line reloadedThirdLine = reloadedPhone.getLine(2);
        
        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/dbdata/AddLineExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[line_id]", reloadedThirdLine.getPrimaryKey());        
        expectedRds.addReplacementObject("[null]", null);
        
        ITable expected = expectedRds.getTable("line");                
        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");
        
        Assertion.assertEquals(expected, actual);        
    }
    
    public void testSave() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/dbdata/EndpointSeed.xml");

        Phone phone = m_context.loadPhone(new Integer(1));
        assertEquals(0, phone.getLines().size());
        User user = m_core.loadUserByDisplayId("testuser");

        Line line = phone.createLine(new LineData());
        LineData lineMeta = line.getLineData();
        lineMeta.setUser(user);
        lineMeta.setFolder(m_context.loadRootLineFolder());
        phone.addLine(line);
        m_context.storePhone(phone);
        
        // reload data to get updated ids
        m_context.flush();        
        Phone reloadedPhone = m_context.loadPhone(new Integer(1));
        Line reloadedLine = reloadedPhone.getLine(0);
        
        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/dbdata/SaveLineExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[null]", null);
        expectedRds.addReplacementObject("[line_id]", reloadedLine.getPrimaryKey());
        ITable expected = expectedRds.getTable("line");
                
        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");
        
        Assertion.assertEquals(expected, actual);        
    }
    
    public void testLoadAndDelete() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/dbdata/LineSeed.xml");
        
        Phone phone = m_context.loadPhone(new Integer(1));
        Collection lines = phone.getLines();
        assertEquals(1, lines.size());        
        lines.clear();
        m_context.storePhone(phone);
        
        Phone cleared = m_context.loadPhone(new Integer(1));
        assertEquals(0, cleared.getLines().size());        
    }
    
    public void testMoveLine() throws Exception {
         TestHelper.cleanInsert("dbdata/ClearDb.xml");
         TestHelper.cleanInsertFlat("phone/dbdata/MoveLineSeed.xml");

         Phone phone = m_context.loadPhone(new Integer(1));
         Line l1 = phone.getLine(0);
         Object[] ids = new Object[] { l1.getPrimaryKey() };
         DataCollectionUtil.moveByPrimaryKey(phone.getLines(), ids, 1);
         m_context.storePhone(phone);
         m_context.flush();
         
         IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/dbdata/MoveLineExpected.xml");
         ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
         expectedRds.addReplacementObject("[null]", null);
         
         ITable expected = expectedRds.getTable("line");                
         ITable actual = TestHelper.getConnection().createDataSet().getTable("line");
         
         Assertion.assertEquals(expected, actual);        
    }
    
    /**
     * Makes sure the line's settings get deleted too
     */
    public void testDeleteLinesWithSettings() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/dbdata/DeleteLineWithSettingsSeed.xml");
        
        Phone phone = m_context.loadPhone(new Integer(1));
        Collection lines = phone.getLines();
        assertEquals(3, lines.size());
        DataCollectionUtil.removeByPrimaryKey(lines, new Object[] {new Integer(2)});
        m_context.storePhone(phone);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/dbdata/DeleteLineWithSettingsExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[null]", null);
        
        ITable expected = expectedRds.getTable("line");                
        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");
        
        Assertion.assertEquals(expected, actual);                
    }
}
