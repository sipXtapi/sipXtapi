/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/cache/dbconnectionimpl/DBCPConnectionPool.java#6 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.sds.cache.dbconnectionimpl;

import com.pingtel.pds.sds.cache.DatabaseConnectionFactory;

import java.sql.SQLException;
import java.sql.Connection;

import org.apache.commons.dbcp.PoolableConnectionFactory;
import org.apache.commons.dbcp.PoolingDataSource;
import org.apache.commons.dbcp.ConnectionFactory;
import org.apache.commons.dbcp.DriverManagerConnectionFactory;
import org.apache.commons.pool.ObjectPool;
import org.apache.commons.pool.impl.GenericObjectPool;


public class DBCPConnectionPool  implements DatabaseConnectionFactory {

    private PoolingDataSource m_dataSource;
    private String m_driverClassname;

    public void initialize( String driverClassname,
                            String jdbcURL,
                            String userName,
                            String password)
            throws SQLException {

        m_driverClassname = driverClassname;

        try {
            Class.forName( m_driverClassname );
        }
        catch (ClassNotFoundException e) {
            throw new RuntimeException ( e.getMessage() );
        }

        ObjectPool connectionPool = new GenericObjectPool(null, 10 );
            ((GenericObjectPool) connectionPool).setTestOnBorrow( true );
            ((GenericObjectPool) connectionPool).setTestOnReturn( true );

        //
        // Next, we'll create a ConnectionFactory that the
        // pool will use to create Connections.
        // We'll use the DriverManagerConnectionFactory,
        // using the connect string passed in the command line
        // arguments.
        //

        // probably want a more configurable way of doing this.
        ConnectionFactory connectionFactory =
                new DriverManagerConnectionFactory( jdbcURL,
                                                    userName, // db username
                                                    password ); // db password



        //
        // Now we'll create the PoolableConnectionFactory, which wraps
        // the "real" Connections created by the ConnectionFactory with
        // the classes that implement the pooling functionality.
        //

        //////////////////////////////////////////////////////////////////////////////////
        //
        // From the DBCP javadoc
        //
        // connFactory - the ConnectionFactory from which to obtain base Connections
        // pool - the ObjectPool in which to pool those Connections
        // stmtPoolFactory - the KeyedObjectPoolFactory to use to create
        //        KeyedObjectPools for pooling PreparedStatements, or null to disable
        //        PreparedStatement pooling
        // validationQuery - a query to use to validate Connections.
        //        Should return at least one row. May be null
        // defaultReadOnly - the default "read only" setting for borrowed Connections
        // defaultAutoCommit - the default "auto commit" setting for returned Connections
        //
        /////////////////////////////////////////////////////////////////////////////////
        PoolableConnectionFactory poolableConnectionFactory = null;

        try {
            poolableConnectionFactory =
                    new PoolableConnectionFactory(  connectionFactory,
                                                    connectionPool,
                                                    null,
                                                    null,
                                                    false,
                                                    true);
        }
        catch (Exception e) {
            throw new RuntimeException ( e.getMessage() );
        }

        //
        // Finally, we create the PoolingDriver itself,
        // passing in the object pool we created.
        //
        m_dataSource = new PoolingDataSource(connectionPool);
    }

    public Connection getConnection() throws SQLException {
        return m_dataSource.getConnection();
    }

    public String getDriverClassname () {
        return m_driverClassname;
    }

}
