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

package com.pingtel.pds.pgs.common;

import java.sql.Connection;
import java.sql.SQLException;

import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.sql.DataSource;

/**
 * Database Support Class - isolates Database specific
 * information in a single place.
 * @author John P. Coffey
 */
public class DBSupport {

    //////////////////////////////////////////////////////////////////////////////
    // Constants
    ////

    // The supported database types
    private static final int UNKNOWN    =-1;
    private static final int ORACLE     = 0;
    private static final int POSTGRESQL = 1;
    private static final int UNSUPPORTED= 2;

    // the Vendor type of the database is just for Indeces right now
    private static int m_type = DBSupport.UNKNOWN;

    // Singleton Construction
    private static DBSupport m_dbSupport = new DBSupport();

    //////////////////////////////////////////////////////////////////////////////
    // Attributes
    ////


    //////////////////////////////////////////////////////////////////////////////
    // Construction
    ////

    //////////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    /**
     * singleton accessor method
     * @return instance of the DBSupport object
     */
    public static DBSupport getInstance() {
        return m_dbSupport;
    }


    /**
     * Vendor Specific Sequence Query SQL strings here
     * It would be nice if we had absolutely no sequence support to query
     * it from another sequence table that contains all sequences
     *
     * @param sequenceName incicated the name of the sequence string
     */
    public synchronized String getNextSeqQueryString ( String sequenceName )
           throws java.sql.SQLException
    {
        // query the database type using JNDI
        if ( m_type == DBSupport.UNKNOWN )
            getVendorInfo();

        if (m_type == DBSupport.ORACLE)
           return "SELECT " + sequenceName + ".NEXTVAL FROM DUAL";
        else
           return "SELECT NEXTVAL ( '" + sequenceName + "' )";
    }


    //////////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////

    /**
     * This Method uses JNDI to query the Connection Name and also
     * the type of Database that we need to connect to.
     *
     * @exception java.sql.SQLException
     */
    private void getVendorInfo ()
        throws java.sql.SQLException {

        // Initialize to null so that we may do cleanip in the finally block
        Connection con = null;
        try {
            InitialContext ctx = new InitialContext();
            // @JC Need to remove the hard coding from here
            DataSource ds = (DataSource)ctx.lookup ( "java:/PDSDataSource" );
            con = ds.getConnection();
            String driverName = con.getMetaData().getDriverName().toLowerCase();
            if ( driverName.indexOf("oracle") >= 0 ) {
                m_type = DBSupport.ORACLE;
            } else if (driverName.indexOf("postgresql") >= 0) {
                m_type = DBSupport.POSTGRESQL;
            } else {
                m_type = DBSupport.UNSUPPORTED;
            }
        } catch ( NamingException ne ) {
            throw new SQLException(ne.getMessage());
        } finally {
            if (con != null)
               con.close();
        }
    }

}

