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

package com.pingtel.pds.pgs.common.ejb;

import com.pingtel.pds.common.JDBCStatementHelper;
import com.pingtel.pds.pgs.common.DBSupport;

import javax.ejb.EJBException;
import javax.naming.InitialContext;
import javax.sql.DataSource;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;

/**
 * This is the base class for all session beans in the system.
 * @author IB,JC
 */
abstract public class JDBCAwareEJB extends BaseEJB {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    private DataSource m_dataSource = null;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////
    protected JDBCAwareEJB () {}

    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    /**
     * This method handles prepared update/insert statements.
     *
     * @param sqlString SQL statement (in the format of a SQL string used
     * in a PreparedStatement) to be used in the update
     * @param parameters array of argument values to be substituted into
     * the sqlString
     * @return number of rows updated
     * @throws SQLException
     */
    protected int executePreparedUpdate ( String sqlString, Object [] parameters )
        throws SQLException {

        Connection con = null;
        int returnValue = 0;

        try {
            con = getConnection();
            returnValue = JDBCStatementHelper.executePreparedUpdate(con, sqlString, parameters);
        }
        finally {
            if (con != null) {
                con.close();
            }
        }

        return returnValue;
    }

    /**
     * executes a query against our database and returns a list of the
     * returned rows.
     *
     * @param sqlString SQL statement (in the format of a SQL string used
     * in a PreparedStatement) to be used in the query
     * @param parameters parameters array of argument values to be substituted into
     * the sqlString.
     * @param numColumns the number of columns that should be returned from the
     * query.  This must match the number of bind variables used in the sqlString
     * @param maxRows the number of rows that you want returned from the query
     * @return an ArrayList of ArrayLists.  Each ArrayList in the returned ArrayList
     * corresponds to a row from the query.
     * @throws SQLException for application errors.
     */
    protected ArrayList executePreparedQuery(   String sqlString,
                                                Object [] parameters,
                                                int numColumns,
                                                int maxRows )
        throws SQLException {

        Connection con = null;
        ArrayList resultSet = new ArrayList();

        try {
            con = getConnection();
            resultSet = JDBCStatementHelper.executePreparedQuery(con, sqlString, parameters, numColumns, maxRows);
            return resultSet;
        }
        finally {
            if (con != null) {
                con.close();
            }
        }
    }


    /**
     * Fetches the Next Sequence Number in a JDBC specific manner
     *
     * @param sequenceName
     * @return
     * @exception EJBException this is fatal as the DB has problems
     * unlike the duplicatekey or create exceptions
     */
    protected int getNextSequenceValue ( String sequenceName )
            throws EJBException  {

        Connection con = null;
        PreparedStatement psSeq = null;
        ResultSet rsSeq = null;
        try {
            con = getConnection();
            String query =
                DBSupport.getInstance().
                    getNextSeqQueryString(sequenceName);
            psSeq = con.prepareStatement( query );
            rsSeq = psSeq.executeQuery();
            rsSeq.next();
            int newID = rsSeq.getInt (1);
            return newID;
        } catch (SQLException se) {
            logFatal( se.toString(), se );
            throw new EJBException (se.getMessage());
        } finally {
            closeJDBCObjects ( con,
                               psSeq,
                               rsSeq );
        }
    }

    /**
     * closes various JDBC Objects
     * @param con
     * @param ps
     * @param rs
     * @exception EJBException
     */
    protected void closeJDBCObjects ( Connection con,
                                      PreparedStatement ps,
                                      ResultSet rs )
    throws EJBException {

        closeJDBCObjects (  con, ps, rs, null, null );

    }

    /**
     * closes various JDBC Objects
     * @param con
     * @param ps
     * @param rs
     * @param sps
     * @param srs
     * @exception EJBException
     */
    protected void closeJDBCObjects ( Connection con,
                                      PreparedStatement ps,
                                      ResultSet rs,
                                      PreparedStatement sps,
                                      ResultSet srs )
        throws EJBException
    {
        String problemMessage = null;

        // Closse Potentially Open Result Set
        try {
            if ( rs != null ) {
                rs.close();
            }
        } catch ( SQLException se ) {
            problemMessage = se.toString();
        }

        try {
            if ( ps != null ) {
                ps.close();
            }
        } catch ( SQLException se ) {
            // @JC Accumulate Error Information
            problemMessage += se.toString();
        }

        try {
            if ( srs != null ) {
                srs.close();
            }
        } catch ( SQLException se ) {
            problemMessage += se.toString();
        }

        try {
            if ( sps != null ) {
                sps.close();
            }
        } catch ( SQLException se ) {
            problemMessage += se.toString();
        }

        try {
            if ( con != null ) {
                con.close();
            }
        } catch ( SQLException se ) {
            problemMessage = se.toString();
        }

        if ( problemMessage != null ) {
            EJBException ejbe = new EJBException ( problemMessage );
            logFatal( problemMessage, ejbe );
            throw ejbe;
        }
    }


    /**
     * wraps the given input string with CDATA markup.
     *
     * @param source String to be wrapped
     * @return wrapped CDATA String
     */
    protected String CDATAIt ( String source ) {
        return "<![CDATA[" + source + "]]>";
    }


    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////

    /**
     * Looks up JNDI DB Connection
     *
     * @return
     * @exception EJBException
     */
    private Connection getConnection () throws EJBException {
        try {
            if ( m_dataSource == null ) {
                InitialContext ctx = new InitialContext();
                m_dataSource = (DataSource)ctx.lookup ( "java:/PDSDataSource" );
            }
            return m_dataSource.getConnection();
        } catch ( Exception e) {
            logFatal( e.toString(), e );
            throw new EJBException ( e.toString() );
        }
    }


    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}
