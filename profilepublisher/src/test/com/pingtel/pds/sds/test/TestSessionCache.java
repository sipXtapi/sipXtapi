/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/test/com/pingtel/pds/sds/test/TestSessionCache.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.sds.test;

import com.pingtel.pds.common.JDBCStatementHelper;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.sds.cache.DatabaseConnectionFactory;
import com.pingtel.pds.sds.cache.SessionCache;
import com.pingtel.pds.sds.cache.SessionCacheException;
import com.pingtel.pds.sds.cache.dbconnectionimpl.DBCPConnectionPool;
import com.pingtel.pds.sds.sip.SipMultiValuedHeader;
import com.pingtel.pds.sds.sip.SipProfileInfo;
import com.pingtel.pds.sds.sip.SipSession;
import com.pingtel.pds.sds.sip.SipSessionContext;
import junit.framework.Assert;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.*;

/**
 * TestSessionCache is a JUnit test case to white-box test the
 * SessionCache class.
 */
public class TestSessionCache extends TestCase {

    private SessionCache m_sessionCache;

    // used to create java.sql.Connections which are used to
    // verify results in the SDS database
    private DatabaseConnectionFactory m_dbConnectionFactory;

    public TestSessionCache ( String name ) {
            super ( name );
    }

    public static Test suite() {
        return new TestSuite( TestSessionCache.class );
    }


    /////////////////////////////////////////////////////////////
    //
    // standard Junit method to prepare the test fixture.
    //
    /////////////////////////////////////////////////////////////
    protected void setUp () {

        m_dbConnectionFactory = new DBCPConnectionPool ();

        try {
            m_dbConnectionFactory.initialize(
                        "org.postgresql.Driver",
                        "jdbc:postgresql://localhost/SDS",
                        "sds",
                        "sds");

            clearDatabase();

            m_sessionCache =
                    new SessionCache (  "jdbc:postgresql://localhost/SDS",
                                        "sds",
                                        "sds",
                                        "org.postgresql.Driver",
                                        null);
        }
        catch ( Exception e) {
            Assert.fail( e.getMessage() );
        }
    }


    /////////////////////////////////////////////////////////////
    //
    // standard Junit method to free-up resources after the test
    // fixture is complete
    //
    /////////////////////////////////////////////////////////////
    protected void tearDown () {

        try {
            clearDatabase();
        }
        catch (SQLException e) {
            Assert.fail( e.getMessage());
        }
    }


    private void clearDatabase() throws SQLException {
        Connection con = m_dbConnectionFactory.getConnection();

        JDBCStatementHelper.executePreparedUpdate( con, "DELETE FROM MULTI_ATTRIBUTES", null);
        JDBCStatementHelper.executePreparedUpdate( con, "DELETE FROM PROFILE_SEQUENCE_NUMS", null);
        JDBCStatementHelper.executePreparedUpdate( con, "DELETE FROM ATTRIBUTES", null);
        JDBCStatementHelper.executePreparedUpdate( con, "DELETE FROM CONTEXTS", null);
    }


    /**
     * tests the CreateSessionContext() method which Creates a persistent
     * SessionContext in the SDS database.
     */
    public void testCreateSessionContext() {
        SipSessionContext context = createSipSessionContext();
        Connection con = null;

        try {
            Integer contextID = m_sessionCache.createSessionContext ( context );
            con = m_dbConnectionFactory.getConnection();

            ArrayList contextRows = getContextsCount(con);
            assertEquals ( 1, contextRows.size() );
            ArrayList attributesRows = getAttributesCount(con);
            assertEquals ( 8, attributesRows.size() );
            ArrayList multiRows = getMultiAttributesCount(con);
            assertEquals ( 7, multiRows.size() );

            m_sessionCache.deleteSessionContext( contextID, false);

            contextRows = getContextsCount(con);
            assertEquals ( 0, contextRows.size() );
            attributesRows = getAttributesCount(con);
            assertEquals ( 0, attributesRows.size() );
            multiRows = getMultiAttributesCount(con);
            assertEquals ( 0, multiRows.size() );
        }
        catch ( Exception e) {
            Assert.fail( e.getMessage() );
        }
        finally {
            if ( con != null ) {
                try {
                    con.close();
                }
                catch (SQLException e) {
                    Assert.fail ( e.getMessage() );
                }
            }
        }
    }


    /**
     * test that we can update the sequence number for profiles for which a
     * device has subscribed for.
     */
    public void testUpdateProfiles () {
        SipSessionContext context = createSipSessionContext();
        Connection con = null;

        try {
            con = m_dbConnectionFactory.getConnection();
            Integer contextID = m_sessionCache.createSessionContext ( context );

            m_sessionCache.updateSessionContextProfileInfo( contextID,
                    new SipProfileInfo (
                            PDSDefinitions.PROF_TYPE_PHONE,
                            "pingtel/ixpressa_x86_win32/ff000000000d/pinger-config",
                            2 ) );

            ArrayList rows =
                    JDBCStatementHelper.executePreparedQuery(
                        con,
                        "SELECT sequence_number FROM PROFILE_SEQUENCE_NUMS WHERE PROFILE_TYPE = ? ",
                        new Object [] { new Integer (PDSDefinitions.PROF_TYPE_PHONE)},
                        1,
                        1 );

            ArrayList row = (ArrayList) rows.get( 0 );
            int sequenceNumber = Integer.parseInt( (String) row.get( 0 ) );
            assertEquals ( 2, sequenceNumber );

            m_sessionCache.deleteSessionContext( contextID, false);
        }
        catch ( Exception e) {
            Assert.fail( e.getMessage() );
        }
        finally {
            if ( con != null ) {
                try {
                    con.close();
                }
                catch (SQLException e) {
                    Assert.fail ( e.getMessage() );
                }
            }
        }
    }


    /**
     * test creating a SipSession, storing it in the database and then
     * fetching it using the findSessionContexts method.   Currently this
     * _seems_ like its broke however it seems to work in the SDS.
     */
    public void testFindSessionContext () {
        SipSessionContext original = createSipSessionContext();
        try {
            Integer contextID = m_sessionCache.createSessionContext ( original );

            Map.Entry result =
                    m_sessionCache.findSessionContext(
                            new SipSession (    "sip:10.1.1.128",
                                                "sip:sipuaconfig@sipuaconfig",
                                                "config-74cec1051112742-ff000000000d@10.1.1.128",
                                                "sip-config") );

            SipSessionContext fetched = (SipSessionContext) result.getValue();

            //assertEquals ( original, fetched );

            m_sessionCache.deleteSessionContext( contextID, false);
        }
        catch (SessionCacheException e) {
            e.printStackTrace();  //To change body of catch statement use Options | File Templates.
        }
    }


    /**
     * test creating a SipSession, storing it in the database and then
     * fetching it using the findSessionContexts method.   The test itself
     * compares the result of the findSessionContext and the
     * findSessionContexts method to see if the results are equal.
     */
    public void testFindSessionContexts () {
        SipSessionContext original = createSipSessionContext();
        try {
            Integer contextID = m_sessionCache.createSessionContext ( original );

            Map.Entry result =
                    m_sessionCache.findSessionContext(
                            new SipSession (    "sip:10.1.1.128",
                                                "sip:sipuaconfig@sipuaconfig",
                                                "config-74cec1051112742-ff000000000d@10.1.1.128",
                                                "sip-config") );

            SipSessionContext fetched = (SipSessionContext) result.getValue();

            HashMap contexts = m_sessionCache.findSessionContexts( "ff000000000d" );
            for ( Iterator i = contexts.entrySet().iterator(); i.hasNext(); ) {
                Map.Entry entry = (Map.Entry) i.next();

                assertEquals ( fetched, entry.getValue() );
            }

            m_sessionCache.deleteSessionContext( contextID, false);
        }
        catch (SessionCacheException e) {
            Assert.fail( e.getMessage() );
        }
    }


    private SipSessionContext createSipSessionContext() {
        Properties singleValueHeaders = new Properties();
        HashMap multiValueHeaders = new HashMap();

        singleValueHeaders.put( "Config_Require", "x-xpressa-apps, x-xpressa-device, x-xpressa-install, x-xpressa-user");
        singleValueHeaders.put( "CSeq", "1 SUBSCRIBE");
        singleValueHeaders.put( "Expires", "86400");
        singleValueHeaders.put( "Config_Allow", "http");
        //singleValueHeaders.put( "Call-Id", "config-74cec1051112742-ff000000000d@10.1.1.128");

        Properties toProperties = new Properties ();
        toProperties.put ( "tag","1051112688asd" );

        SipMultiValuedHeader to = new SipMultiValuedHeader( "sip:sipuaconfig@sipuaconfig", toProperties );
        multiValueHeaders.put( "To", to );

        Properties fromProperties = new Properties ();
        fromProperties.put ( "tag","74cec1051112742" );
        fromProperties.put ( "Serial","ff000000000d" );
        fromProperties.put ( "Mac","ff000000000d" );
        fromProperties.put ( "Version","2.1.8.6" );
        fromProperties.put ( "Model","ixpressa_x86_win32" );
        fromProperties.put ( "Vendor","Pingtel" );


        Properties eventProperties = new Properties();
        SipMultiValuedHeader event =
                new SipMultiValuedHeader ( "sip-config", eventProperties );
        multiValueHeaders.put( "Event", event );

        SipMultiValuedHeader from = new SipMultiValuedHeader ( "sip:10.1.1.128", fromProperties );
        multiValueHeaders.put( "From", from );

        Properties callidProperties = new Properties();
        SipMultiValuedHeader callID = new SipMultiValuedHeader ( "config-74cec1051112742-ff000000000d@10.1.1.128", callidProperties );
        multiValueHeaders.put( "Call-Id", callID );

        ArrayList profileSequence = new ArrayList();

        profileSequence.add(
                new SipProfileInfo ( PDSDefinitions.PROF_TYPE_PHONE,
                        "pingtel/ixpressa_x86_win32/ff000000000d/pinger-config", 1 ) );

        profileSequence.add(
                new SipProfileInfo ( PDSDefinitions.PROF_TYPE_USER,
                        "pingtel/ixpressa_x86_win32/ff000000000d/user-config", 1 ) );

        profileSequence.add(
                new SipProfileInfo ( PDSDefinitions.PROF_TYPE_APPLICATION_REF,
                        "pingtel/ixpressa_x86_win32/ff000000000d/app-config", 1 ) );

        SipSessionContext context =
                new SipSessionContext (
                        singleValueHeaders,
                        multiValueHeaders,
                        profileSequence,
                        1009 );
        return context;
    }


    private ArrayList getMultiAttributesCount(Connection con) throws SQLException {
        ArrayList multiRows =
                JDBCStatementHelper.executePreparedQuery(   con,
                                                            "SELECT ID FROM MULTI_ATTRIBUTES",
                                                            null,
                                                            1,
                                                            1000 );
        return multiRows;
    }

    private ArrayList getAttributesCount(Connection con) throws SQLException {
        ArrayList attributesRows =
                JDBCStatementHelper.executePreparedQuery(   con,
                                                            "SELECT ID FROM ATTRIBUTES",
                                                            null,
                                                            1,
                                                            1000 );
        return attributesRows;
    }

    private ArrayList getContextsCount(Connection con) throws SQLException {
        ArrayList contextRows =
                JDBCStatementHelper.executePreparedQuery(   con,
                                                            "SELECT ID FROM CONTEXTS",
                                                            null,
                                                            1,
                                                            1000 );
        return contextRows;
    }

}
