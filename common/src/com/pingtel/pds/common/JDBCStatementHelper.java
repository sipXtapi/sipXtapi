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
package com.pingtel.pds.common;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;

/**
 * JDBCStatementHelper provides wrappers or high-level primitives
 * for JDBC access.   It is thread safe.
 */
public class JDBCStatementHelper {

    private static Boolean m_debugOn = new Boolean ( false );


    /**
     * if debugging is turned of then the SQL statements that are
     * executed will be output to the console.
     *
     * @param debug if true then output will occur.
     */
    static public void setDebug ( boolean debug) {
        synchronized ( m_debugOn ) {
            m_debugOn = new Boolean ( debug );
        }
    }

    static private boolean getDebug () {
        synchronized ( m_debugOn ) {
            return m_debugOn.booleanValue();
        }
    }

    /**
     * executes a prepared statement similar to that of PreparedStatment.executeUpdate()
     * and returns the number of rows affected.   It handles the binding of the query
     * parameters and the creation and closing of all the associated objects required.
     *
     * @param con connection to the database that the statement is to be issued
     * against
     * @param sqlString SQL statement to run in standard JDBC PreparedStatement
     * format
     * @param parameters bind variables to be substituted into the SQL statement
     * @return number of rows affected in the database
     * @throws SQLException is thrown for all errors.
     */
    static public int executePreparedUpdate (   Connection con,
                                                String sqlString,
                                                Object [] parameters )
        throws SQLException {

        PreparedStatement ps = null;

        if ( sqlString == null || sqlString.trim().length() == 0 ) {
            throw new IllegalArgumentException ( "sqlString can not be null or blank");
        }

        if ( con == null ) {
            throw new IllegalArgumentException ( "con can not be null");
        }

        if ( getDebug() ) {
            printDebugSQLString(sqlString, parameters);
        }

        try {
            if ( parameters != null ) {
                if ( !parametersContainNulls ( parameters ) ) {
                    ps = con.prepareStatement ( sqlString );

                    //parameter indexing starts at 1 rather than 0
                    if ( parameters != null ) {
                        for (int i = 0; i < parameters.length; i++ ) {
                            ps.setObject( i+1, parameters [ i ]);
                        }
                    }

                    return ps.executeUpdate();
                }
                else {
                    Statement stmt = con.createStatement();
                    String basicSQL = makeStatementBasic ( sqlString, parameters );
                    return stmt.executeUpdate( basicSQL );
                }
            }
            else {
                ps = con.prepareStatement ( sqlString );
                return ps.executeUpdate();
            }
        }
        finally {
            ArrayList closeObjects = new ArrayList();
            if ( ps != null ) {
                closeObjects.add( ps );
            }

            closeJDBCObjects ( closeObjects );
        }
    }


    /**
     * executes a prepared statement similar to that of PreparedStatment.executeQuery()
     * and returns the number of rows affected.   It handles the binding of the query
     * parameters and the creation and closing of all the associated objects required.
     *
     * @param con connection to the database that the statement is to be issued
     * against
     * @param sqlString sqlString SQL statement to run in standard JDBC PreparedStatement
     * format
     * @param parameters parameters bind variables to be substituted into the SQL statement
     * @param numColumns number of columns or returned values that will be returned
     * from the sqlString
     * @param maxRows the maximum number of rows that should be returned from the
     * query
     * @return an ArrayList of ArrayLists.  Each ArrayList in the returned value
     * represents a row returned by the sqlStatement
     * @throws SQLException
     */
    static public ArrayList executePreparedQuery( Connection con,
                                           String sqlString,
                                           Object [] parameters,
                                           int numColumns,
                                           int maxRows )
        throws SQLException {

        if ( sqlString == null || sqlString.trim().length() == 0 ) {
            throw new IllegalArgumentException ( "sqlString can not be null or blank");
        }

        if ( con == null ) {
            throw new IllegalArgumentException ( "con can not be null");
        }

        if ( getDebug() ) {
            printDebugSQLString(sqlString, parameters);
        }

        PreparedStatement ps = null;
        ResultSet rs = null;
        ArrayList resultSet = new ArrayList();

        try {
            ps = con.prepareStatement ( sqlString );

            if ( parameters != null ) {
                // parameter indexing starts at 1 rather than 0
                for (int i=0; (parameters != null) && (i < parameters.length); i++){
                    ps.setObject( i+1, parameters [ i ]);
                }
            }

            // Execute the prepared statment
            rs = ps.executeQuery();

            while (rs.next()) {

                ArrayList row = new ArrayList();

                for (int i=0; i < numColumns; i++ ) {
                    row.add( i, rs.getString( i + 1 ) );
                }

                // Append the resultant row to resultset. note that
                // ideally we could use the metadata of the result set to
                // verify whether we are doing sensible things with this
                // object, instead leat the SQL exception handle this
                resultSet.add( row );

                // Terminate the search early if maxRows reached
                // Zero or negative means complete query
                if ( (maxRows > 0) && (resultSet.size() >= maxRows) )
                    break;
            }
            return resultSet;
        }
        finally {
            ArrayList closeObjects = new ArrayList();
            if ( rs != null ){
                closeObjects.add( rs );
            }
            if ( ps != null ) {
                closeObjects.add( ps );
            }
            closeJDBCObjects ( closeObjects );
        }
    }

    private static void printDebugSQLString(String sqlString, Object[] parameters)
            throws SQLException {
        System.out.println ( "=======================================");
        System.out.println ( convertSQLToString ( sqlString, parameters ));
        System.out.println ( "=======================================");
    }


    static private boolean parametersContainNulls ( Object [] parameters ){

        boolean needsSubstitutions = false;

        for ( int i = 0; i < parameters.length; ++i ) {
            if ( parameters [ i ] == null ) {
                needsSubstitutions = true;
                break;
            }
        }

        return needsSubstitutions;
    }


    static private String makeStatementBasic ( String sqlString, Object [] parameters ){

        StringBuffer newSQL = new StringBuffer();
        int counter = 0;
        char [] sqlChars = sqlString.toCharArray();

        for ( int i = 0 ; i < sqlChars.length; ++i ) {
            if ( sqlChars [ i ] == '?'  )  {

                if ( parameters [ counter ] == null ) {
                    newSQL.append( "NULL" );
                }
                else {
                    if ( parameters [ counter ] instanceof String )
                        newSQL.append('\'')
                            .append ( parameters [ counter ].toString() )
                                .append( '\'' );
                    else
                        newSQL.append ( parameters [ counter ].toString() );
                }

                ++counter;
            }
            else
                newSQL.append ( sqlChars [ i ] );
        }

        return newSQL.toString();
    }


    static protected void closeJDBCObjects ( List objectsToClose )
            throws SQLException {

        for ( Iterator i = objectsToClose.iterator(); i.hasNext(); ) {
            Object o = i.next();

            if ( o instanceof Statement ) {
                ((Statement) o).close();
            }
            else if ( o instanceof ResultSet ) {
                ((ResultSet) o).close();
            }
        }
    }



    /**
     * Helper method to print out the SQL String as opposed to the one with
     * prepared sql ?'s in it
     */
    private static String convertSQLToString ( String preparedSQLString, Object[] args)
        throws SQLException {

        String result;
        StringTokenizer toker = new StringTokenizer( preparedSQLString, "?" );
        if (args == null) {
            result =  preparedSQLString;
        }
        else if ( toker.countTokens() == args.length + 1 ) {
            StringBuffer sb = new StringBuffer();
            int argIndex = 0;
            while ( toker.hasMoreElements() ) {
                sb.append( toker.nextToken() );
                // Last token is special as there is not arg for it
                if (toker.hasMoreElements()) {
                    sb.append( "'" + args[argIndex++].toString() + "'");
                }
            }
            result = sb.toString();
        }
        else {
            throw new SQLException (
                "Prepared Statement: " +
                preparedSQLString +
                " expects " + toker.countTokens() +
                " parameters, it was passed: " + args.length );
        }
        return result;
    }

}
