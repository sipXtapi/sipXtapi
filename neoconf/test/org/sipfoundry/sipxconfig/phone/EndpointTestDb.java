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

import java.io.InputStream;

import org.dbunit.Assertion;
import org.dbunit.DatabaseTestCase;
import org.dbunit.database.IDatabaseConnection;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.dbunit.dataset.filter.DefaultColumnFilter;
import org.dbunit.dataset.xml.FlatXmlDataSet;
import org.sipfoundry.sipxconfig.TestHelper;


public class EndpointTestDb extends DatabaseTestCase {
    
    protected IDatabaseConnection getConnection() throws Exception
    {
        return TestHelper.getDbUnitConnection();
    }
    
    protected IDataSet getDataSet() throws Exception {
        // no setup required
        return loadDataSet("endpoint-save-expected.xml");
    }
    
    public void testSave() throws Exception {
        PhoneContext context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        
        Endpoint e = new Endpoint();
        e.setSerialNumber("999123456");
        e.setName("unittest-sample phone1");
        context.storeEndpoint(e);
        
        IDataSet actualDs = getConnection().createDataSet();
        ITable actualRawTable = actualDs.getTable("endpoint");
        ITable actualTable = DefaultColumnFilter.excludedColumnsTable(
                actualRawTable, new String[] { "endpoint_id" } );        
                
        ReplacementDataSet ds = loadDataSet("endpoint-save-expected.xml");               
        ITable expectedTable = ds.getTable("endpoint");
                
        Assertion.assertEquals(expectedTable, actualTable);
    }
    
    private ReplacementDataSet loadDataSet(String fileResource) throws Exception {
        InputStream datasetStream = getClass().getResourceAsStream(fileResource);
        FlatXmlDataSet flatXmlDs = new FlatXmlDataSet(datasetStream);
        return new ReplacementDataSet(flatXmlDs);
    }
}
