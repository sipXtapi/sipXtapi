/**
 *
 * Copyright (c) 2003 Pingtel Corp.  (work in progress)
 *
 * This is an unpublished work containing Pingtel Corporation's confidential
 * and proprietary information.  Disclosure, use or reproduction without
 * written authorization of Pingtel Corp. is prohibited.
 *
 * $File: //depot/OPENDEV/sipXconfig/common/src/test/com/pingtel/pds/common/test/TestJDBCStatementHelper.java $
 * $Revision: #1 $
 * Author: 
 */

package com.pingtel.pds.common.test;

import junit.framework.Assert;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.ArrayList;

import com.pingtel.pds.common.JDBCStatementHelper;

/**
 * TestJDBCStatementHelper is a JUnit test case.  It is designed to test the
 * JDBCStatementHelper class.
 *
 * @author ibutcher
 * 
 */
public class TestJDBCStatementHelper extends TestCase{

    static {
        try {
            Class.forName("org.postgresql.Driver");
        }
        catch (ClassNotFoundException e) {
            Assert.fail(e.getMessage());
        }
    }

//////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final String CREATE_TABLE_SQL_STRING =
            "CREATE TABLE TEST_TABLE (name VARCHAR(20), age INTEGER)";

    private static final String DROP_TABLE_SQL_STRING =
            "DROP TABLE TEST_TABLE";

    private static final String FULL_INSERT_SQL_STRING =
            "INSERT INTO TEST_TABLE (name,age) VALUES (?,?)";

    private static final String FULL_SELECT_SQL_STRING =
            "SELECT name, age FROM TEST_TABLE";

    private static final String AGE_ONLY_INSERT_SQL_STRING =
            "INSERT INTO TEST_TABLE (age) VALUES (?)";

    private static final String AGE_ONLY_SELECT_SQL_STRING =
            "SELECT age FROM TEST_TABLE";

    private static final int MAX_ROWS = 1000;


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Connection mConn; // used for DB access


//////////////////////////////////////////////////////////////////////////
// Construction
////
    public TestJDBCStatementHelper ( String name ) {
        super ( name );
        try {
            DriverManager.registerDriver(new org.postgresql.Driver());
            mConn = DriverManager.getConnection(
                    "jdbc:postgresql://localhost/PDS",
                    "postgres",
                    "password");
        }
        catch (SQLException e) {
             e.printStackTrace();
            Assert.fail(e.getMessage());
        }
    }


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * standard Junit method to create the suite of tests.   It adds all
     * of the test cases in this class in order.
     */
    public static Test suite() {
        return new TestSuite( TestJDBCStatementHelper.class );
    }

    public void testCreateTable() {
        try {
            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    CREATE_TABLE_SQL_STRING,
                    null);

            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    DROP_TABLE_SQL_STRING,
                    null);
        }
        catch (SQLException e) {
            Assert.fail( e.getMessage() );
        }

    }

    public void testFullInsert() {
        try {
            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    CREATE_TABLE_SQL_STRING,
                    null);

            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    FULL_INSERT_SQL_STRING,
                    new Object [] {"ian", new Integer("31")});

            ArrayList rows =
                    JDBCStatementHelper.executePreparedQuery(
                            mConn,
                            FULL_SELECT_SQL_STRING,
                            null,
                            2,
                            MAX_ROWS);

            assertEquals (1, rows.size());
            ArrayList row = (ArrayList) rows.iterator().next();
            String name = (String) row.get(0);
            String age = (String) row.get(1);

            assertEquals ("ian",name);
            assertEquals ("31",age);

            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    DROP_TABLE_SQL_STRING,
                    null);
        }
        catch (SQLException e) {
            Assert.fail( e.getMessage() );
        }
    }


    public void testPartialInsert() {
        try {
            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    CREATE_TABLE_SQL_STRING,
                    null);

            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    AGE_ONLY_INSERT_SQL_STRING,
                    new Object [] {new Integer("31")});

            ArrayList rows =
                    JDBCStatementHelper.executePreparedQuery(
                            mConn,
                            AGE_ONLY_SELECT_SQL_STRING,
                            null,
                            1,
                            1000);

            assertEquals (1, rows.size());
            ArrayList row = (ArrayList) rows.iterator().next();
            String age = (String) row.get(0);

            assertEquals ("31",age);

            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    DROP_TABLE_SQL_STRING,
                    null);
        }
        catch (SQLException e) {
            Assert.fail( e.getMessage() );
        }
    }


    public void testUpdate() {
        try {
            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    CREATE_TABLE_SQL_STRING,
                    null);

            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    "INSERT INTO TEST_TABLE (name,age) VALUES (?,?)",
                    new Object [] {"ian", new Integer("31")});

            ArrayList rows =
                    JDBCStatementHelper.executePreparedQuery(
                            mConn,
                            FULL_SELECT_SQL_STRING,
                            null,
                            2,
                            1000);

            assertEquals (1, rows.size());
            ArrayList row = (ArrayList) rows.iterator().next();
            String name = (String) row.get(0);
            String age = (String) row.get(1);

            assertEquals ("ian",name);
            assertEquals ("31",age);

            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    "UPDATE TEST_TABLE SET age = ? WHERE NAME = ?",
                    new Object [] {new Integer("99"),"ian"});

            rows.clear();
            row.clear();

            rows =
                    JDBCStatementHelper.executePreparedQuery(
                            mConn,
                            FULL_SELECT_SQL_STRING,
                            null,
                            2,
                            1000);

            assertEquals (1, rows.size());
            row = (ArrayList) rows.iterator().next();
            name = (String) row.get(0);
            age = (String) row.get(1);

            assertEquals ("ian",name);
            assertEquals ("99",age);

            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    DROP_TABLE_SQL_STRING,
                    null);
        }
        catch (SQLException e) {
            Assert.fail( e.getMessage() );
        }
    }


    public void testDelete() {
        try {
            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    CREATE_TABLE_SQL_STRING,
                    null);

            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    FULL_INSERT_SQL_STRING,
                    new Object [] {"ian", new Integer("31")});

            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    FULL_INSERT_SQL_STRING,
                    new Object [] {"dan", new Integer("46")});

            ArrayList rows =
                    JDBCStatementHelper.executePreparedQuery(
                            mConn,
                            FULL_SELECT_SQL_STRING,
                            null,
                            2,
                            1000);

            assertEquals (2, rows.size());

            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    "DELETE FROM TEST_TABLE WHERE name = ?",
                    new Object [] {"dan"});

            rows.clear();
            rows =
                    JDBCStatementHelper.executePreparedQuery(
                            mConn,
                            FULL_SELECT_SQL_STRING,
                            null,
                            2,
                            1);

            assertEquals (1, rows.size());

            JDBCStatementHelper.executePreparedUpdate(
                    mConn,
                    DROP_TABLE_SQL_STRING,
                    null);
        }
        catch (SQLException e) {
            Assert.fail( e.getMessage() );
        }
    }


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    /**
     *  standard Junit method to prepare the test fixture.
     */
    protected void setUp () {
        try {
            clearDatabase();
        }
        catch ( Exception e) {
            //do nothing, this happens when you drop the table.
        }
    }


    /**
     *  standard Junit method to free-up resources after the test
     *  fixture is complete
     */
    protected void tearDown () {
        try {
            mConn.close();
        }
        catch (SQLException e) {
            Assert.fail( e.getMessage());
        }
    }

    private void clearDatabase() throws SQLException {
        JDBCStatementHelper.executePreparedUpdate( mConn, DROP_TABLE_SQL_STRING, null);
    }


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////    


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
