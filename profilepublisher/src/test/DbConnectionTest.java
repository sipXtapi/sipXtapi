/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

import junit.framework.TestCase;

import com.pingtel.pds.sds.cache.DatabaseConnectionFactory;
import com.pingtel.pds.sds.cache.dbconnectionimpl.DBCPConnectionPool;
import java.sql.Connection;

/**
 * Connect to DB using SDS connection pooling
 */
public class DbConnectionTest extends TestCase 
{
    String m_driverClassname = "org.postgresql.Driver";

    String m_userId = "postgres";

    String m_password = "";
    
    /**
     * Connect and disconnect from SDS database using DB connection 
     * management
     */
    public void testSdsConnection()
    {
        try 
        {
            DatabaseConnectionFactory db = new DBCPConnectionPool();
            String url = "jdbc:postgresql://localhost/SDS";
            db.initialize(m_driverClassname,
                          url,
                          m_userId,
                          m_password);

            Connection c = db.getConnection();

            assertTrue(c != null);
            System.out.println("SDS connection ok");
        }
        catch (Throwable e) 
        {
            e.printStackTrace(System.out);
            fail("Connection to database");
        }
    }

    /**
     * Connect and disconnect from SDS database using DB connection 
     * management
     */
    public void testPdsConnection()
    {
        try 
        {
            DatabaseConnectionFactory db = new DBCPConnectionPool();
            String url = "jdbc:postgresql://localhost/PDS";
            db.initialize(m_driverClassname,
                          url,
                          m_userId,
                          m_password);

            Connection c = db.getConnection();

            assertTrue(c != null);
            System.out.println("PDS connection ok");
        }
        catch (Throwable e) 
        {
            e.printStackTrace(System.out);
            fail("Connection to database");
        }
    }
}
