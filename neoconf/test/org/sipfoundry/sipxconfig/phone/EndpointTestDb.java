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
import org.dbunit.operation.DatabaseOperation;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.vendor.Polycom;


public class EndpointTestDb extends DatabaseTestCase {
    
    static {
//        System.setProperty("xml.catalog.files", "test/catalog.xml");
//        System.setProperty("relative-catalogs", "yes");
//        System.setProperty("static-catalog", "yes");
//        System.setProperty("catalog-class-name", "org.apache.xml.resolver.Resolver");
        System.setProperty("verbosity", "99");    
        System.setProperty("org.xml.sax.driver", "org.apache.xerces.parsers.SAXParser");
    }
    
    protected IDatabaseConnection getConnection() throws Exception
    {
        return TestHelper.getDbUnitConnection();
    }

    protected DatabaseOperation getSetUpOperation() throws Exception
    {
        return DatabaseOperation.NONE;
    }

    protected DatabaseOperation getTearDownOperation() throws Exception
    {
        return DatabaseOperation.NONE;
    }
    
    protected IDataSet getDataSet() throws Exception {
        // empty, no setup required, (null not accepted)
        return null;
    }
    
    public void testSave() throws Exception {
        PhoneContext context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        
        Endpoint e = new Endpoint();
        e.setPhoneId(Polycom.MODEL_300.getModelId());
        e.setSerialNumber("999123456");
        e.setName("unittest-sample phone1");
        //context.storeEndpoint(e);
        
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
