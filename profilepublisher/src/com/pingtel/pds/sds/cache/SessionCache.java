/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/cache/SessionCache.java#6 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.sds.cache;

import com.pingtel.pds.common.DeviceNameUtil;
import com.pingtel.pds.common.JDBCStatementHelper;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.sds.cache.dbconnectionimpl.DBCPConnectionPool;
import com.pingtel.pds.sds.sip.*;
import org.apache.log4j.Category;
import org.apache.log4j.NDC;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.*;

/**
 * SessionCache maintains the state of enrolled devices.   Creates a separate
 * thread (Housekeeper) which periodically wakes up and removes any expired
 * subscriptions.
 */
public class SessionCache {

    // log4j logging class
    private static Category m_logger;

    static {
        m_logger = Category.getInstance( "pds" );
    }

    // DatabaseConnectionFactory is an interface which is used as an
    // abstraction for how to obtain database connections.
    private DatabaseConnectionFactory m_dbConnectionFactory;

    // flag used to shutdown the housekeeper thread
    private boolean m_shutdown = false;

    // every housekeeper wakes up once every minute
    private final int m_sleepIntervalSecs = 60;

    // Create the Attribute row
    private static final String m_createContextAttributeSQL =
        "INSERT INTO attributes ( id, ctx_id, name, value ) " +
        "VALUES (?, ?, ?, ? )";

    private static final String m_createProfileSequenceSQL =
        "INSERT INTO profile_sequence_nums " +
        "(contexts_id, profile_type, sequence_number, url) " +
        "VALUES (?, ?, ?, ?)";

    // Store the worker thread for cleanup (join method)
    private Thread m_housekeeperThread = null;

    /**
     * constructor
     * @param databaseURL the JDBC URL for the persistent storage database
     * @param databaseUserID database user in the schema described above
     * @param databasePassword password for database user
     * @param driverClassname fully qualified class name for the class to
     * be used to create database connections.
     * @param dbConnectionFactoryClassname fully qualified class name for the
     * DatabaseConnectionFactory implementation.  If none is supplied then
     * DBCPConnectionPool is used.
     * @throws SessionCacheException for all exceptions.
     * @see DBCPConnectionPool
     */
    public SessionCache( String databaseURL,
                         String databaseUserID,
                         String databasePassword,
                         String driverClassname,
                         String dbConnectionFactoryClassname )
        throws SessionCacheException {

        m_logger.debug( "creating SessionCache: database URL " + databaseURL +
                        "user ID " + databaseUserID + " database driver " +
                        dbConnectionFactoryClassname );


        if ( dbConnectionFactoryClassname == null ) {
            m_dbConnectionFactory = new DBCPConnectionPool ();
        }
        else {
            try {
                m_dbConnectionFactory =
                        (DatabaseConnectionFactory)
                            Class.forName( dbConnectionFactoryClassname).newInstance();
            }
            catch ( Exception e) {
                throw new SessionCacheException ( "problem creating connection pool factory", e );
            }
        }

        try {
            m_dbConnectionFactory.initialize( driverClassname, databaseURL, databaseUserID, databasePassword );
        }
        catch (SQLException e) {
            throw new SessionCacheException ( "problem setting up connection pool", e );
        }

        // Launch a housekeeper thread to purge expired contexts
        m_logger.debug( "Creating Housekeeper" );
        m_housekeeperThread = new Thread ( new icHouseKeeper( m_dbConnectionFactory ) );
        m_logger.debug( "Starting Housekeeper" );
        m_housekeeperThread.start();
        m_logger.debug( "Housekeeper Thread Started" );
    }


    /**
     * shutdown the SessionCache - specifically shutdown the housekeeper
     */
    public void shutdown() {
        m_shutdown = true;
        m_logger.debug( "SessionCache Shutting down..." );
        if (m_housekeeperThread != null) {
            m_housekeeperThread.interrupt();
            try { m_housekeeperThread.join(1000); }
            catch (InterruptedException ex){}
        }
    }


    /**
     * Creates a persistent SessionContext.   The method extracts all of the
     * SIP headersand writes them to the database.
     *
     * @param context the SipSessionContext that you want to persist
     * @return subcriptionContextID (PK in db)
     * @throws SessionCacheException
     */
    public Integer createSessionContext ( SipSessionContext context )
        throws SessionCacheException {
        Integer contextID = null;
        Connection con = null;
        try {
            m_logger.debug( "Creating Context" );
            con = m_dbConnectionFactory.getConnection();

            try {
                // Perform the JDBC as a transaction
                con.setAutoCommit( false );

                // Create primary key for the SessionContext
                contextID = getNextSequenceValue("CONTEXTS_SEQ");

                Object[] args = new Object[2];
                args[0] = contextID;

                // Store the expiration period as seconds since 1970
                args[1] = new Integer ( (int)(System.currentTimeMillis() / 1000) +
                                        context.getExpirationSeconds() );

                // Create the master context row with a csequence starting at 1000
                // Create the SessionContext Row
                JDBCStatementHelper.executePreparedUpdate(
                        con,
                        "INSERT INTO contexts ( id, sip_expires, sip_output_cseq ) " +
                        "VALUES (?, ?, 1000 )",
                        args );

                // Call a piece of common code to create the Clild Rows
                // this common code will be used in the update case also
                createSessionContextChildRows(contextID, context, con);

                // Commit changes to DB
                con.commit();
            } catch ( SQLException ex ) {
                m_logger.error( "Error creating Context", ex );

                // Note that rolling back the transaction will
                // not affect the sequence number, this does not matter
                // since this sequence number will be unused
                if ( con != null ) {
                    con.rollback();
                }
            } finally {
                if ( con != null ) {
                    con.setAutoCommit( true );
                    con.close();
                }

            }
        } catch (Exception ex) {
            throw new SessionCacheException (ex.getMessage());
        }
        return contextID;
    }


    private Integer getNextSequenceValue ( String sequenceName ) throws SQLException {

        String sequenceValue = null;
        Connection con = null;
        con = m_dbConnectionFactory.getConnection();
        String queryString = null;

        if ( m_dbConnectionFactory.getDriverClassname().equals( "oracle.jdbc.driver.OracleDriver" ) ) {
            queryString = "SELECT " + sequenceName + ".NEXTVAL FROM DUAL";
        }
        else if ( m_dbConnectionFactory.getDriverClassname().equals( "org.postgresql.Driver" ) ) {
            queryString =  "SELECT NEXTVAL ( '" + sequenceName + "' )";
        }
        else {
            // assert
        }

        try {
            ArrayList rows =
                    JDBCStatementHelper.executePreparedQuery(   con,
                                                                queryString,
                                                                null,
                                                                1,
                                                                1 );

            ArrayList row = (ArrayList) rows.get( 0 );
            sequenceValue = (String) row.get( 0 );
        }
        finally {
            if ( con != null ) {
                con.close();
            }
        }

        return new Integer ( sequenceValue );
    }


    /**
     * Updates the SessionContext with the updated profile info from the profile writer
     * and also updates the outbound CSequence number so that teh next restore from the DB
     * will have the next CSequence number
     *
     * @param contextID PK for context to be updated
     * @param updatedProfileInfo profile URL, type and sequence number to be written to
     * the database
     * @exception SessionCacheException
     */
    public void updateSessionContextProfileInfo ( Integer contextID,
                                                  SipProfileInfo updatedProfileInfo )
        throws SessionCacheException {
        // get a free pool entry from the pool manager
        try {
            m_logger.debug( "Updating Context" );
            Connection con = null;
            con = m_dbConnectionFactory.getConnection();

            try {
                // Updates always done under a transaction
                con.setAutoCommit( false );
                // find the matching Attributes
                Object[] args = new Object[4];
                args[1] = updatedProfileInfo.getURL();
                args[0] = new Integer (updatedProfileInfo.getProfileSequenceNumber());
                args[2] = new Integer (updatedProfileInfo.getProfileType());
                args[3] = contextID;

                JDBCStatementHelper.executePreparedUpdate (
                        con,
                        "UPDATE profile_sequence_nums SET " +
                        "sequence_number=?, url=? " +
                        "WHERE (profile_type = ? AND contexts_id = ?)",
                        args );

                // Updates just the CSequence number associated with the context
                JDBCStatementHelper.executePreparedUpdate (
                        con,
                        "UPDATE contexts SET sip_output_cseq=sip_output_cseq + 1 WHERE (contexts.id= ?)",
                        new Object [] { contextID }  );

                m_logger.debug( "Incrementing Cseq in SDS database" );
                con.commit();
            }
            catch (SQLException ex) {
                if ( con != null ){
                    con.rollback();
                }
                m_logger.error( "Exception occurred updating Context", ex);
                throw new SessionCacheException( ex.getMessage() );
            }
            finally {
                if ( con != null ) {
                    con.setAutoCommit(true);
                    con.close();
                }
            }
        }
        catch (Exception ex) {
            throw new SessionCacheException( ex.getMessage() );
        }
    }

    /**
     * Finds the single matching Session context in the database
     *
     * @return a Map.Entry of a context ID and a SipSessionContext
     */
    public Map.Entry findSessionContext( SipSession session )
        throws SessionCacheException  {

        try {
            m_logger.debug( "Looking up Context using Session" );
            // get a free pool entry from the pool manager
            Connection con = null;
            try {
                con = m_dbConnectionFactory.getConnection();

                // Initialize the result set to null so
                // it can be cleaned up in the finally block

                Object[] args = new Object[4];
                args[0] = session.getToAddress();
                args[1] = session.getFromAddress();
                args[2] = session.getCallId();
                args[3] = session.getEvent();

                // Execute the outer query to find the Context ID
                //This query is primarily used to return all the contexts associated
                //with a Session (To, From and CallID) (and event maybe)

                ArrayList rows = JDBCStatementHelper.executePreparedQuery(
                        con,
                        "SELECT id, sip_expires, sip_output_cseq FROM contexts "+
                        "WHERE id = (SELECT ctx_id FROM attributes attr "+
                        "WHERE (attr.name = 'To' AND attr.value = ?) "+
                        "INTERSECT SELECT ctx_id FROM attributes attr "+
                        "WHERE (attr.name = 'From' AND attr.value = ?) "+
                        "INTERSECT SELECT ctx_id FROM attributes attr "+
                        "WHERE (attr.name = 'Call-Id' AND attr.value = ?) "+
                        "INTERSECT SELECT ctx_id FROM attributes attr "+
                        "WHERE (attr.name = 'Event' AND attr.value = ?) )",
                        args,
                        3,
                        100000 );

                Integer contextID = null;
                Properties singleValuedSipHeaders = new Properties();

                // This is a collection of MultiValiedHeaders
                HashMap multiValuedSipHeaders = new HashMap();

                // Should only be single row here if the Context Exists
                // however the design allows us to return multiple
                for ( Iterator iRow = rows.iterator(); iRow.hasNext(); ) {
                    // Read in the All important ID column, this will be used later
                    // to determine all the other fields
                    ArrayList row = (ArrayList) iRow.next();
                    contextID = new Integer (  (String) row.get(0));

                    // the new outgoing sequence number needs to be unique each
                    // and every time we generate a SIP message
                    int outputCseq = Integer.parseInt((String) row.get(2));

                    ArrayList attributesRows = null;
                    Object[] args1 = new Object[1];
                    args1[0] = contextID;

                    // run an inner query to find the name value pairs
                    // of all the attributes both single valued and those
                    // with url parameters

                    // This query is used to construct a Context from the DB
                    attributesRows = JDBCStatementHelper.executePreparedQuery(
                            con,
                            "SELECT id, name, value FROM attributes attr WHERE attr.ctx_id = ?",
                            args1,
                            3,
                            10000 );

                    for ( Iterator iAttributesRows = attributesRows.iterator(); iAttributesRows.hasNext(); ) {
                        // Read in the SIP Fields and ID
                        ArrayList attributesRow = (ArrayList) iAttributesRows.next();
                        Integer attrID = new Integer ( (String) attributesRow.get( 0 ));
                        //Integer attrID = new Integer ( rs1.getInt("id") );

                        String sipHeaderName = (String) attributesRow.get( 1 );
                        String sipHeaderValue = (String) attributesRow.get( 2 );
                        boolean multiValuedHeader = false;
                        // Determine whether this is a special multivalued
                        // field or single valued
                        if ( sipHeaderName.equalsIgnoreCase(SipMessage.SIP_FROM_ADDRESS ) ||
                             sipHeaderName.equalsIgnoreCase(SipMessage.SIP_TO_ADDRESS ) ||
                             sipHeaderName.equalsIgnoreCase(SipMessage.SIP_CALL_ID ) ||
                             sipHeaderName.equalsIgnoreCase(SipMessage.SIP_EVENT ) ) {
                             multiValuedHeader = true;
                        }
                        // at this stage the name and value could be either
                        // simple or composite multivalued
                        //Object[] args2 = new Object[1];
                        //args2[0] = attrID;

                        ArrayList multiRows =
                                JDBCStatementHelper.executePreparedQuery(
                                    con,
                                    "SELECT name, value FROM multi_attributes ma WHERE ma.attr_id = ?",
                                    new Object [] {attrID},
                                    2,
                                    10000 );

                        Properties urlParameters = null;
                        for ( Iterator iMultiRows = multiRows.iterator(); iMultiRows.hasNext(); ) {
                            ArrayList multiRow = (ArrayList) iMultiRows.next();
                            if (urlParameters == null)
                                urlParameters = new Properties ();
                            // technically these are called url parameters
                            String urlParameterName = (String) multiRow.get( 0 );
                            String urlParameterValue = (String) multiRow.get( 1 );
                            // Append the url parameters to the list
                            urlParameters.setProperty( urlParameterName,
                                                       urlParameterValue);
                        }

                        // check whether this was a simple or multivalued line
                        if ( multiValuedHeader == true ) {
                            // Compound type, note tat the
                            // url parameter field can be null
                            SipMultiValuedHeader entry =
                                new SipMultiValuedHeader (
                                    sipHeaderValue,
                                    urlParameters);
                            multiValuedSipHeaders.put(sipHeaderName, entry);
                        } else {
                            // Simple type
                            singleValuedSipHeaders.setProperty(
                                sipHeaderName, sipHeaderValue);
                        }

                    }

                    // Fetch the sequence number information from the DB
                    ArrayList profileSeqRows =
                    JDBCStatementHelper.executePreparedQuery(
                            con,
                            "SELECT profile_type, sequence_number, url FROM profile_sequence_nums WHERE contexts_id = ?",
                            new Object [] { contextID },
                            3,
                            10000 );

                    ArrayList profileInfo = null;
                    for ( Iterator profileSeqRowsI = profileSeqRows.iterator(); profileSeqRowsI.hasNext(); ) {
                        ArrayList profileSeqRow = (ArrayList) profileSeqRowsI.next();
                        if (profileInfo == null)
                            profileInfo = new ArrayList();
                        int profileType = Integer.parseInt( (String)profileSeqRow.get( 0 ) );
                        String url = (String)profileSeqRow.get( 2 );
                        int sequenceNumber = Integer.parseInt( (String)profileSeqRow.get( 1 ) );
                        SipProfileInfo sipProfileInfo =
                            new SipProfileInfo(profileType, url, sequenceNumber);
                        profileInfo.add(sipProfileInfo);
                    }

                    // Create the SessionContext from here
                    SipSessionContext context =
                        new SipSessionContext (
                                singleValuedSipHeaders,
                                multiValuedSipHeaders,
                                profileInfo,
                                outputCseq );
                    // Return the single Cache Entry
                    HashMap resultMap = new HashMap(1);
                    resultMap.put(contextID, context);
                    Iterator it = resultMap.entrySet().iterator();
                    return (Map.Entry)it.next();
                } // for
            }
            finally {
                if ( con != null ) {
                    con.close();
                }
            }
        }
        catch (SQLException ex) {
            m_logger.error( "Error finding Context" , ex );
            throw new SessionCacheException ( ex.getMessage() );
        }
        // should not return here
        return null;
    }

    /**
     * Finds all active Session contexts associated with this phone.
     * The phone is identified by its mac address and that mac address
     * may have many active sessions associated with it.
     *
     * @param macAddress used to identify the enrolled device contexts
     * @return a collection of contexts associated with this mac address
     * this is a Hashtable with the ContextID as the key and the
     *
     * @exception SessionCacheException
     */
    public HashMap findSessionContexts ( String macAddress )
        throws SessionCacheException  {
        // now for the real Database work, initialize the context to null
        // as there may be no context
        HashMap deviceContexts = new HashMap();

        m_logger.debug ( "Finding Contexts for MAC:" + macAddress ) ;
        Connection con = null;

        try {
            con = m_dbConnectionFactory.getConnection();

            // Execute the query for the Session Context
            ArrayList rows =
                JDBCStatementHelper.executePreparedQuery(
                        con,
                        "SELECT ctx_id, name, value from attributes WHERE attributes.ctx_id = " +
                        "(SELECT attr.ctx_id FROM attributes attr, multi_attributes mattr " +
                        "WHERE mattr.name like 'Mac' and mattr.value like ? AND mattr.attr_id = attr.id )" +
                        "AND (attributes.name like 'From'  OR " +
                        "     attributes.name like 'To'    OR " +
                        "     attributes.name like 'Event' OR " +
                        "     attributes.name like 'Call-Id')" +
                        "ORDER BY ctx_id, name, value",
                        new Object [] { macAddress },
                        3,
                        10000 );

            HashMap dbqueryResults = new HashMap();

            for ( Iterator iRows = rows.iterator(); iRows.hasNext(); ) {
                ArrayList row = (ArrayList) iRows.next();

                Integer ctxID = new Integer ( (String) row.get( 0 ) );
                String name = (String) row.get( 1 );
                String value = (String) row.get( 2 );
                if (!dbqueryResults.containsKey( ctxID ))
                    dbqueryResults.put(ctxID, new Properties());
                Properties properties = (Properties)dbqueryResults.get(ctxID);
                properties.setProperty(name, value);
            }

            // Create Session Identifiers that we can use to look up
            // a single session from
            for ( Iterator iter = dbqueryResults.values().iterator(); iter.hasNext(); ) {
                Properties props = (Properties) iter.next();
                SipSession session = new SipSession (
                    props.getProperty(SipMessage.SIP_FROM_ADDRESS),
                    props.getProperty(SipMessage.SIP_TO_ADDRESS),
                    props.getProperty(SipMessage.SIP_CALL_ID),
                    props.getProperty(SipMessage.SIP_EVENT));

                // Find the single DB Subacription Context
                // associated with this session
                Map.Entry contextEntry =
                    findSessionContext ( session );

                // and add it to the list
                deviceContexts.put(
                    contextEntry.getKey(),
                    contextEntry.getValue());
            }
        }
        catch (SQLException ex) {
            m_logger.error( "Error finding Context for MAC" , ex );
            throw new SessionCacheException ( ex.getMessage() );
        }
        finally {
            if ( con != null ) {
                try {
                    con.close();
                }
                catch (SQLException e) { }
            }
        }
        return deviceContexts;
    }


    /**
     * Updates the sequence number
     *
     * @param contextID
     * @param newEnrollmentLease
     *
     * @return
     * @exception SessionCacheException
     */
    public int updateSessionContextSequenceAndLease (
        Integer contextID,
        Integer newEnrollmentLease )
        throws SessionCacheException {

        // now for the real Database work
        Object[] args  = new Object[2];
        args[0] = new Integer ( (int)(System.currentTimeMillis()/1000) +
                                      newEnrollmentLease.intValue() );
        args [1] = contextID;

        try {
            Connection con = null;

            try {
                con = m_dbConnectionFactory.getConnection();
                m_logger.debug( "Updating Cseq in DB" );

                //Updates the sequence number in the context for all the sessions
                //associated with the phone.
                return JDBCStatementHelper.
                    executePreparedUpdate(
                            con,
                            "UPDATE contexts SET " +
                            "sip_output_cseq=sip_output_cseq + 1, sip_expires = ? " +
                            "WHERE (contexts.id= ?)",
                            args );

            }
            finally {
                if ( con != null ){
                    con.close();
                }
            }
        } catch ( Exception ex ) {
            throw new SessionCacheException ( ex.getMessage() );
        }
    }

    /**
     * Replace all the SessionContext fields, this is initially going to be
     * the brute force approach to updating a coupld of fields in the DB Context
     * this is going to become more intelligent over time.
     *
     * @param contextID PK of database session row to replace
     * @param context SIP context object
     *
     * @exception SessionCacheException
     */
    public void updateSessionContext ( Integer contextID,
                                       SipSessionContext context )
        throws SessionCacheException {

        // Delete all child tables leaving the master context row
        // NB there is a potential problem here with transaction of the db
        deleteSessionContext( contextID, true );

        try {
            Connection con = null;

            try {
                con = m_dbConnectionFactory.getConnection();
                con.setAutoCommit(false);
                createSessionContextChildRows( contextID, context, con );
                // Commit changes to DB
                con.commit();
            }
            catch ( SQLException ex ) {
                m_logger.error( "error creating context", ex );

                // Note that rolling back the transaction will
                // not affect the sequence number, this does not matter
                // since this sequence number will be unused
                if ( con != null ) {
                    con.rollback();
                }
            }
            finally {
                if ( con != null ) {
                    con.setAutoCommit( true );
                    // Note releasing the PoolEntry restores the autocommit mode
                    // to the connectino defaults (true)
                    con.close();
                }
            }
        }
        catch (Exception ex) {
            throw new SessionCacheException (ex.getMessage());
        }
    }



    private void createSessionContextChildRows ( Integer contextID,
                                                 SipSessionContext context,
                                                 Connection con )
        throws SQLException {
        // The SIPSessionContext consists of Multi and Single valued headers
        // each of which has its own peculiarities.
        HashMap multiValuedSipHeaders = context.getMultiValuedSipHeaders();
        Properties singleValuedSipHeaders = context.getSingleValuedSipHeaders();

        // Iterate through all of the sipheader: value lines
        Enumeration enum = singleValuedSipHeaders.keys();

        while ( enum.hasMoreElements() ) {
            Integer attrID = getNextSequenceValue( "ATTRIBUTES_SEQ" );
            String name = (String) enum.nextElement();
            String value = singleValuedSipHeaders.getProperty(name);

            JDBCStatementHelper.executePreparedUpdate(
                    con,
                    m_createContextAttributeSQL,
                    new Object [] { attrID, contextID, name, value} );
        }

        // Go through each of the MultiValuedHeader objects
        // and persist it to the attributes and multi-attribute joined
        // tables, reuse the args above
        Iterator sipHeaderIterator = multiValuedSipHeaders.keySet().iterator();

        while ( sipHeaderIterator.hasNext() ) {
            String headerName =  (String)sipHeaderIterator.next();

            SipMultiValuedHeader mvh =
                (SipMultiValuedHeader)multiValuedSipHeaders.get( headerName );

            // get a new key for the attribute table (this will
            // also be used as a foreign key in the many attributes table
            Integer attrID = getNextSequenceValue ( "ATTRIBUTES_SEQ" );

            JDBCStatementHelper.executePreparedUpdate(
                    con,
                    m_createContextAttributeSQL,
                    new Object [] {attrID, contextID, headerName, mvh.getPrimaryValue()} );

            Properties urlParameters = mvh.getNvPairs();

            if (urlParameters != null) {
                enum = urlParameters.keys();
                while ( enum.hasMoreElements() ) {
                    String name = (String)enum.nextElement();
                    String value = urlParameters.getProperty(name);
                    Integer multiAttrID = getNextSequenceValue ( "MULTI_ATTRIBUTES_SEQ" );

                    // Create the Attribute row
                    JDBCStatementHelper.executePreparedUpdate(
                            con,
                            "INSERT INTO multi_attributes ( id, attr_id, name, value ) " +
                            "VALUES (?, ?, ?, ? )",
                            new Object [] {multiAttrID, attrID, name, value } );
                }
            }
        }

        // Depending on the version of the SIP Device the
        // SUBSCRIBE message will contain an optional field
        // SIP_SUBSCRIBE_ALLOWED_PROTOCOLS or
        // SIP_SUBSCRIBE_ALLOWED_PROTOCOLS1 (in the days when
        // it was mis-spelt!).  If this header is present we need to
        // parse for the individual requested profiles and insert
        // a row in the PROFILE_SEQUENCE_NUMS initializing its seq
        // uence number to -1
        SipMultiValuedHeader eventEntry =
            (SipMultiValuedHeader)multiValuedSipHeaders.
                get(SipMessage.SIP_EVENT);

        // The event type is either the legacy one which includes 4 profiles in one
        // or is is the new style subscribe event where we only have one profile
        // per event
        if ( eventEntry != null ) {
            String eventType = eventEntry.getPrimaryValue();
            ArrayList requestedProfiles = new ArrayList(4);
            // The old legacy version contains multple requests in one
            if ( eventEntry.getPrimaryValue().equalsIgnoreCase(SipMessage.SIP_LEGACY_SUBSCRIBE_EVENT) ) {
                // This should list a comma separated list of up to 4 profiles
                String configRequired = singleValuedSipHeaders.
                    getProperty( SipMessage.SIP_SUBSCRIBE_REQUIRED_PROFILES );

                if ( configRequired != null ) {
                    // Parse the config-require line which looks like the following in the
                    // old format 'Config_require: x-xpressa-apps, x-xpressa-device, x-xpressa-install, x-xpressa-user'
                    StringTokenizer toker = new StringTokenizer(configRequired, ",");
                    while ( toker.hasMoreElements() ) {
                        String requestedProfileName = toker.nextToken().trim();
                        if ( requestedProfileName.equalsIgnoreCase("x-xpressa-apps") ) {
                            requestedProfiles.add(
                                new Integer( PDSDefinitions.PROF_TYPE_APPLICATION_REF ) );
                        } else if ( requestedProfileName.equalsIgnoreCase("x-xpressa-device") ) {
                            requestedProfiles.add(
                                new Integer( PDSDefinitions.PROF_TYPE_PHONE ) );
                        } else if ( requestedProfileName.equalsIgnoreCase("x-xpressa-install") ) {
                            requestedProfiles.add(
                                new Integer( PDSDefinitions.PROF_TYPE_UPGRADESCRIPT ) );
                        } else if ( requestedProfileName.equalsIgnoreCase("x-xpressa-user") ) {
                            requestedProfiles.add(
                                new Integer( PDSDefinitions.PROF_TYPE_USER ) );
                        } else {
                            m_logger.debug ( "Unrecognized Request Type: " +
                                             requestedProfileName + " in subscribe request header" );
                        }
                    }
                }
            } else {
                // new SIP Std identifies this as the eventType - much simpler
                requestedProfiles.add( eventType );
            }

            Object[] args2 = new Object[4];
            args2[0] = contextID;

            // If we have profile info associated with the context
            // insert the rows into the database, othwerwise do nothing
            // until we have a projection, in this scheme
            // only when the url's do not respond we put a -1 in for the
            // sequence number (indicating temporarily down)
            ArrayList sequenceInfo = context.getProfileSequenceInfo();
            if ( sequenceInfo != null ) {
                for ( Iterator iter = sequenceInfo.iterator(); iter.hasNext(); ) {
                    SipProfileInfo nextProfileInfo = (SipProfileInfo) iter.next();
                    args2[1] = new Integer ( nextProfileInfo.getProfileType() );
                    args2[2] = new Integer ( nextProfileInfo.getProfileSequenceNumber() );
                    args2[3] = nextProfileInfo.getURL();
                    //m_logger.debug( "Executing SQL: " +
                    //                convertSQLToString(
                    //                m_createProfileSequenceSQL, args2) );
                    JDBCStatementHelper.executePreparedUpdate (
                            con,
                        m_createProfileSequenceSQL, args2 );
                }

            } else { // Do nothing
                for (Iterator it = requestedProfiles.iterator(); it.hasNext();) {
                    int profileType = ((Integer)it.next()).intValue();
                    args2[1] = new Integer( profileType );
                    args2[2] = new Integer(-1);
                    args2[3] = DeviceNameUtil.getInstance().
                        getDeviceProfileName( profileType,
                                              context.getVendor(),
                                              context.getModel(),
                                              context.getMacAddress() );

                    JDBCStatementHelper.executePreparedUpdate (
                        con, m_createProfileSequenceSQL, args2 );
                }
            }
        }
    }



    /**
     * Delete the Session Context from the Database
     *
     * @param contextID PK of the Context to be deleted
     * @param leaveMasterRow whether or not to remove the top-level
     * row in the CONTEXTS table.   This method is called from a variety
     * of places, in some cases they don't want to remove the master row.
     *
     * @exception SessionCacheException
     */
    public void deleteSessionContext ( Integer contextID, boolean leaveMasterRow )
        throws SessionCacheException {

        try {
            Connection con = null;

            // Cascade Delete each row using jdbc transaction
            // this ensures that all the tables associated
            // with a context ID are either all deleted or none
            try {
                con = m_dbConnectionFactory.getConnection();
                con.setAutoCommit(false);

                ArrayList rows =
                        JDBCStatementHelper.executePreparedQuery (
                                con,
                                "SELECT id FROM attributes WHERE ctx_id = ?",
                                new Object [] { contextID} ,
                                1,
                                10000 );

                Object[] args = new Object[1];
                //Iterate over the attribute rows
                for ( Iterator iRows = rows.iterator(); iRows.hasNext(); ) {
                    ArrayList row = (ArrayList) iRows.next();
                    Integer attributeID = new Integer ( (String) row.get( 0 ) );
                    args[0] = attributeID;
                    // delete all the URL parameter name value
                    // pairs associated with this attribute
                    JDBCStatementHelper.executePreparedUpdate(
                            con,
                            "DELETE FROM multi_attributes WHERE attr_id = ?",
                            new Object [] { attributeID } );
                }

                args[0] = contextID;
                // delete all the Attribute rows for the context
                JDBCStatementHelper.executePreparedUpdate(
                        con,
                        "DELETE FROM attributes WHERE ctx_id = ?",
                        args );

                // delete the profile_sequence table (also uses contextID)
                JDBCStatementHelper.executePreparedUpdate(
                        con,
                        "DELETE FROM profile_sequence_nums WHERE contexts_id = ?",
                        args );

                // distinguish between case where we update and recycle an
                // existing context.  In this case the contextID must remain
                if ( leaveMasterRow == false ) {
                    JDBCStatementHelper.executePreparedUpdate(
                            con,
                            "DELETE FROM contexts WHERE id = ?",
                            args );
                }
                con.commit();
            }
            catch (SQLException ex) {
                if ( con != null ) {
                    con.rollback();
                }
                throw new SessionCacheException( ex.getMessage() );
            }
            finally {
                if ( con != null ) {
                    con.setAutoCommit(true);
                    con.close();
                }
            }
        } catch (Exception ex) {
            throw new SessionCacheException( ex.getMessage() );
        }
    }


    /**
     * This inner class purges expired entries from the SDS Database.
     * The housekeeper thread wakes up periodically to remove expired
     * subscriptions from the session cache.
     *
     */
    class icHouseKeeper implements Runnable {

        DatabaseConnectionFactory m_dbConnectionFactory;

        public icHouseKeeper ( DatabaseConnectionFactory connectionFactory ) {
            m_dbConnectionFactory = connectionFactory;
        }

        /**
         * Worker thread - waits for SessionCache to expire
         * before removing them from the DB
         */
        public void run () {
            Object[] args = new Object[1];
            NDC.pop();
            NDC.push("SDS");
            Thread.currentThread().setName( "housekeeper" );
            NDC.push ( Thread.currentThread().toString() );
            m_logger.debug( "Running Housekeeper's run() method" );


            while ( !m_shutdown ) {
                try {
                    Thread.currentThread().sleep( m_sleepIntervalSecs * 1000 );
                } catch (InterruptedException ex) {
                    if (m_shutdown == true) {
                        // The Thread was shutdown from the RMI Service
                        return;
                    }
                }

                try {
                    // request a free pool entry from the pool manager
                    Connection con = null;

                    try {
                        con = m_dbConnectionFactory.getConnection();
                        // args is current time in seconds since 1970
                        args[0] = new Integer ( (int)(System.currentTimeMillis() / 1000) );
                        //m_logger.debug( "Executing SQL: " +
                        //                convertSQLToString(
                        //                m_findStaleEntriesSQL, args) );

                        /* Purges all stale entries from the SDS database */
                        ArrayList rows = JDBCStatementHelper.executePreparedQuery( con,
                                "SELECT id FROM contexts WHERE sip_expires < ?", args, 1, 1000000 );

                        for ( Iterator iRows = rows.iterator(); iRows.hasNext(); ) {
                            // fetch the context ID and use this to delete
                            // MULTI_ATTRIBUTE, ATTRIBUTES and PROFILE_SEQUENCE_NUMS
                            // that are related to the ContextID
                            ArrayList row = (ArrayList) iRows.next();
                            Integer contextID = new Integer ( (String) row.get( 0 ) );
                            deleteSessionContext( contextID, false );
                        }
                    }
                    finally {
                        if ( con != null ){
                            con.close();
                        }
                    }
                }
                catch (Exception ex) {
                    m_logger.error("icHouseKeeper::run :" + ex.getMessage(), ex);
                }
            }  // while

        }  // run

    }  // icHouseKeeper

}
