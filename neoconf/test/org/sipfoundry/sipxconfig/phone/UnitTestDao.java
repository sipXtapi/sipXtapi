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

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

import javax.sql.DataSource;

import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Construct test data meant to be perisistent in database. Unittest are not meant to alter any
 * data created, but instead, rely on the data here to always be available. If unittests need
 * to alter data, they should create and destroy it themselves.
 * 
 * Biggest reason this class exists is that unittests need to interact with data created in other
 * libraries and there's no api in this library to create it yet, nor should there be until
 * there's a need to do this from the applicaton
 */
public class UnitTestDao extends HibernateDaoSupport {
    
    public static final String APPLICATION_CONTEXT_FILE = 
        "org/sipfoundry/sipxconfig/phone/applicationContext-unittest-dao.xml";

    private DataSource m_dataSource;
    
    private PhoneContext m_phoneContext;

    private int m_testUserId;
    
    private Endpoint m_teardownEndpoint;
    
    private SettingSet m_teardownSettings;
    
    private Credential m_teardownCredential;

    private Line m_teardownLine;    

    /**
     * test that data in database is setup correctly if not, initialize it's values here. should
     * be called in setUp of unittests that interact with database
     */
    public boolean initializeImmutableData() {
        try {
            Connection connection = m_dataSource.getConnection();
            initializeTestTable(connection);

            // do i need to put connection back in pool?
            
        } catch (SQLException e) {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    public void setDataSource(DataSource dataSource) {
        m_dataSource = dataSource;
    }
    
    public DataSource getDataSource() {
        return m_dataSource;
    }
    
    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    /**
     * verify test data is still there and clean, should be called in tearDown of unittests that
     * interact with database.  There should
     */
    public boolean verifyDataUnaltered() {

        // If unittest fails to tear down, next unittest run
        //   may fail due to duplicate data errors
        // Order is important here to comply with foriegn key
        //   relationships
        if (m_teardownLine != null) {
            m_phoneContext.deleteLine(m_teardownLine);
            m_teardownLine = null;
        }
        if (m_teardownEndpoint != null) {
            m_phoneContext.deleteEndpoint(m_teardownEndpoint);
            m_teardownEndpoint = null;
        }
        if (m_teardownSettings != null) {
            m_phoneContext.deleteSetting(m_teardownSettings);
            m_teardownSettings = null;
        }
        if (m_teardownCredential != null) {
            m_phoneContext.deleteCredential(m_teardownCredential);
            m_teardownCredential = null;
        }
        m_phoneContext.flush();
        
        return true;
    }

    /**
     * User created just for testing data
     * 
     * @return
     */
    public int getTestUserId() {
        return m_testUserId;
    }

    private void initializeTestTable(Connection connection) throws SQLException {

        //test of test data table exists
        Statement query = connection.createStatement();
        try {
            boolean initialized = query.executeQuery("SELECT 1 FROM pg_tables where "
                    + "tablename = 'test_data'").first();
            if (!initialized) {
                Statement update = connection.createStatement();
                try {
                    update.executeUpdate("create table test_data "
                            + "(label varchar(256) not null, " + "record_id int4 not null)");
                } finally {
                    update.close();
                }
            }
        } finally {
            query.close();
        }

        initializeTestUser(connection);
    }

    private void initializeTestUser(Connection connection) throws SQLException {
        String testLabel = "createTestUser";
        m_testUserId = getRecordId(testLabel, connection);
        if (m_testUserId == -1) {
            User testUser = new User();
            testUser.setExtension("233212121"); // assumption, unique
            testUser.setPassword("any-password");
            testUser.setFirstName("Test");
            testUser.setLastName("User");
            testUser.setDisplayId("testuser");
            m_phoneContext.saveUser(testUser);
            m_testUserId = testUser.getId();
            setRecordId(testLabel, connection, testUser.getId());
        }
    }
    
    private int getRecordId(String testLabel, Connection connection) throws SQLException {
        int recordId = -1;
        Statement statement = connection.createStatement();
        try {
            boolean initialized = statement.executeQuery("select record_id from " 
                    + "test_data where label = '" + testLabel + "'").first();
            if (initialized) {
                ResultSet results = statement.getResultSet();
                recordId = results.getInt(1);
            }
        } finally {
            statement.close();
        }

        return recordId;
    }

    private void setRecordId(String testLabel, Connection connection, int id) throws SQLException {
        Statement statement = connection.createStatement();
        try {
            statement.execute("insert into test_data " + "(label,record_id) values ('"
                    + testLabel + "', " + id + ")");
        } finally {
            statement.close();
        }
    }
    
    /**
     * Create some generic sample data, destroyed verifyDataUnaltered
     */
    public Endpoint createSampleEndpoint() {
        Endpoint endpoint = new Endpoint();
        // assumption that this is unique
        endpoint.setSerialNumber("f34298760024fcc1"); 
        endpoint.setPhoneId(GenericPhone.GENERIC_PHONE_ID);
        m_phoneContext.storeEndpoint(endpoint);
        m_teardownEndpoint = endpoint;
        
        return endpoint;
    }
    
    /**
     * Create some generic sample data, destroyed verifyDataUnaltered
     */    
    public Credential createSampleCredential() {
        Credential cred = new Credential();
        m_phoneContext.storeCredential(cred);
        m_teardownCredential = cred;
        
        return cred;
    }
    
    /**
     * Create some generic sample data, destroyed verifyDataUnaltered
     */
    public SettingSet createSampleSettingSet() {
        SettingSet root = new SettingSet("root");       
        SettingSet subset = new SettingSet("subset");
        root.addSetting(subset);
        Setting setting = new Setting("subsetting", "value");
        subset.addSetting(setting);

        m_phoneContext.storeSetting(root, PhoneContext.CASCADE);
        m_teardownSettings = root;
                
        return root;
    }
    
    /**
     * Create some generic sample data, destroyed verifyDataUnaltered
     */
    public Line createSampleLine(User user) {
        Line line = new Line();        
        line.setCredential(createSampleCredential());
        line.setEndpoint(createSampleEndpoint());
        line.setSettings(createSampleSettingSet());
        line.setUser(user);
        m_phoneContext.storeLine(line);                
        m_teardownLine = line;
        
        return line;
    }
}