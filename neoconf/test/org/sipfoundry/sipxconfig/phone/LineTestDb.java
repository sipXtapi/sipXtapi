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

import java.util.ArrayList;
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

    private Class m_class = EndpointTestDb.class;

    public void setUp() {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
    }

    public void testSave() throws Exception {
        TestHelper.cleanInsert(m_class, "ClearDb.xml");
        TestHelper.cleanInsertFlat(m_class, "EndpointSeed.xml");

        Endpoint endpoint = m_context.loadEndpoint(1);
        User user = m_context.loadUserByDisplayId("testuser");

        List lines = new ArrayList();
        Line line = new Line();
        lines.add(line);
        line.setUser(user);

        endpoint.setLines(lines);
        m_context.storeEndpoint(endpoint);
        
        IDataSet expectedDs = TestHelper.loadDataSetFlat(m_class, "SaveLineExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[endpoint_id]", new Integer(endpoint.getId()));
        expectedRds.addReplacementObject("[line_id]", new Integer(line.getId()));
        expectedRds.addReplacementObject("[user_id]", new Integer(user.getId()));
        expectedRds.addReplacementObject("[null]", null);
        
        ITable expected = expectedRds.getTable("line");
                
        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");
        
        Assertion.assertEquals(expected, actual);        
    }
}
