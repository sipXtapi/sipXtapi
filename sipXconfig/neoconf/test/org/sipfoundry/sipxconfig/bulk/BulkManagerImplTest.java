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
package org.sipfoundry.sipxconfig.bulk;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class BulkManagerImplTest extends SipxDatabaseTestCase {
    private BulkManager m_bulkManager;

    protected void setUp() throws Exception {
        super.setUp();
        ApplicationContext context = TestHelper.getApplicationContext();
        m_bulkManager = (BulkManager) context.getBean(BulkManager.CONTEXT_BEAN_NAME);               
        TestHelper.cleanInsert("ClearDb.xml");
    }
    
    public void testInsertFromCsvEmpty() throws Exception {
        m_bulkManager.insertFromCsv(new StringReader(""));
        assertEquals(0, getConnection().getRowCount("users"));
        assertEquals(0, getConnection().getRowCount("phone"));
        assertEquals(0, getConnection().getRowCount("line"));
        assertEquals(0, getConnection().getRowCount("user_group"));
        assertEquals(0, getConnection().getRowCount("phone_group"));
    }
    
    public void testInsertFromCsv() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("cutsheet.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(5, getConnection().getRowCount("users"));
        assertEquals(5, getConnection().getRowCount("phone"));
        assertEquals(5, getConnection().getRowCount("line"));
//        assertEquals(2, getConnection().getRowCount("user_group"));
//        assertEquals(2, getConnection().getRowCount("phone_group"));
    }
    
}
