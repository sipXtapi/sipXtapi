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

import java.util.List;

import junit.framework.TestCase;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.TestHelper;

/**
 * You need to call 'ant reset-db-patch' which clears a lot of data in your
 * database. before calling running this test. 
 */
public class LineTestDb extends TestCase {

    private PhoneContext m_context;

    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
    }
    
    protected void tearDown() throws Exception {
    }

    public void testSave() throws Exception {
        TestHelper.setUpHibernateSession();
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/dbdata/EndpointSeed.xml");

        Endpoint endpoint = m_context.loadEndpoint(1);
        assertEquals(0, endpoint.getLines().size());
        User user = m_context.loadUserByDisplayId("testuser");

        Line line = new Line();
        line.setUser(user);
        endpoint.addLine(line);
        m_context.storeEndpoint(endpoint);
        
        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/dbdata/SaveLineExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[line_id]", new Integer(1));        
        expectedRds.addReplacementObject("[null]", null);
        
        ITable expected = expectedRds.getTable("line");
        m_context.flush();
                
        TestHelper.tearDownHibernateSession();
        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");
        
        Assertion.assertEquals(expected, actual);        
    }
    
    public void testAddingLine() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/dbdata/AddLineSeed.xml");

        Endpoint endpoint = m_context.loadEndpoint(1);
        assertEquals(2, endpoint.getLines().size());
        User user = m_context.loadUserByDisplayId("testuser");

        Line secondLine = new Line();
        secondLine.setUser(user);
        endpoint.addLine(secondLine);
        m_context.storeEndpoint(endpoint);
        
        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/dbdata/AddLineExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[line_id]", new Integer(secondLine.getId()));        
        expectedRds.addReplacementObject("[null]", null);
        
        ITable expected = expectedRds.getTable("line");                
        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");
        
        Assertion.assertEquals(expected, actual);        
    }
    
    public void testLoadAndDelete() throws Exception {
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/dbdata/LineSeed.xml");
        
        Endpoint endpoint = m_context.loadEndpoint(1);
        List lines = endpoint.getLines();
        assertEquals(1, lines.size());        
        lines.clear();
        m_context.storeEndpoint(endpoint);
        
        Endpoint cleared = m_context.loadEndpoint(1);
        assertEquals(0, cleared.getLines().size());        
    }
}
