/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/user/ExtensionPoolAdvocateBean.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.pgs.user;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationHome;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;

import javax.ejb.*;
import javax.naming.Context;
import javax.naming.InitialContext;
import java.rmi.RemoteException;
import java.util.*;
import java.sql.SQLException;

/**
 * ExtensionPoolAdvocateBean is the EJ Bean implementation class for the
 * ExtensionPoolAdvocate session bean.   It provides the functionality
 * for all of the ExtensionPool features.
 *
 * @author ibutcher
 *
 */
public class ExtensionPoolAdvocateBean extends JDBCAwareEJB
        implements SessionBean, ExtensionPoolAdvocateBusiness {


    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////

    public SessionContext mCTX;

    // Home references
    private UserHome mUserHome;
    private OrganizationHome mOrganizationHome;


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////

    public ExtensionPoolAdvocateBean() { }

    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    /**
     *  Standard EJB implementation
     */
    public void ejbCreate() throws CreateException { }

    /**
     *  Standard EJB implementation
     */
    public void ejbActivate() throws EJBException {}

    /**
     *  Standard EJB implementation
     */
    public void ejbPassivate() throws EJBException {}

    /**
     *  Standard EJB implementation
     */
    public void ejbRemove() throws EJBException {}

    /**
     *  Standard EJB implementation
     */
    public void setSessionContext(SessionContext ctx) {
        try {
            Context initial = new InitialContext();
            mUserHome = (UserHome) initial.lookup("User");
            mOrganizationHome = (OrganizationHome) initial.lookup( "Organization" );
        }
        catch (Exception ne) {
            logFatal( ne.toString(), ne );
            throw new EJBException(ne);
        }

        mCTX = ctx;
    }

    /**
     * createExtensionPool creates a extension pool (a named set of extensions
     * which can then be selected from by users when they want to assign an
     * extension to a user) for an organization.
     *
     * @param organizationID the PK of the organization that you are creating
     * pool for.
     * @param name the name by which the pool will be known (must be unique
     * within an organization).
     * @return the PK for the new pool.
     * @throws PDSException for validation errors and application errors
     *
     */
    public Integer createExtensionPool ( String organizationID, String name )
            throws PDSException {

        Integer newID = null;
        String externalOrgID = null;

        if ( name != null && name.length() > MAX_NAME_LEN ) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC621",
                                        "E3028",
                                        new Object[]{
                                            "name",
                                            new Integer(MAX_NAME_LEN)}));
        }



        try {
            checkExtensionPoolNameUnique(name);

            Organization organization =
                mOrganizationHome.findByPrimaryKey( Integer.valueOf( organizationID ));

            externalOrgID = organization.getExternalID();

            newID = new Integer ( getNextSequenceValue( "EXTENSION_POOLS_SEQ" ) );

            executePreparedUpdate(  "INSERT INTO EXTENSION_POOLS ( NAME, ORG_ID, ID ) " +
                                    "VALUES ( ?, ?, ? )",
                                    new Object [] { name,
                                                    Integer.valueOf ( organizationID ),
                                                    newID } );
        }
        catch (PDSException e) {
            throw new PDSException ( collateErrorMessages ("UC621"), e);
        }
        catch (Exception e) {
            logFatal ( e.toString() );

            throw new EJBException(
                collateErrorMessages(   "UC621",
                                        "E4079",
                                        new Object[]{ name, externalOrgID } ) );
        }

        return newID;
    }


    /**
     * deletes an existing Extension Pool.
     * To delete an extension pool you must unassign any extensions in
     * that pool from any users which may have them.   This is to remove
     * the RI constraint.   The rest is simple.
     *
     * @param extensionPoolID the (internal) PK of the pool to delete
     * @throws com.pingtel.pds.common.PDSException for application level errors.
     */
    public void deleteExtensionPool ( String extensionPoolID ) throws PDSException {

        try {
            ArrayList rows =
                executePreparedQuery(   "SELECT EXTENSION_NUMBER FROM EXTENSIONS WHERE EXT_POOL_ID = ?",
                                        new Object [] { Integer.valueOf ( extensionPoolID ) },
                                        1,
                                        1000000 );

            for ( Iterator iRows = rows.iterator(); iRows.hasNext(); ) {
                ArrayList row = (ArrayList) iRows.next();

                String extension = (String) row.get( 0 );

                Collection assignedUser = mUserHome.findByExtension( extension );

                for ( Iterator iUser = assignedUser.iterator(); iUser.hasNext(); ) {
                    User user = (User) iUser.next();
                    user.setExtension( null );
                }
            }

            executePreparedUpdate(  "DELETE FROM EXTENSIONS  " +
                                    "WHERE EXT_POOL_ID = ? ",
                                    new Object [] { Integer.valueOf ( extensionPoolID )  } );

            executePreparedUpdate(  "DELETE FROM EXTENSION_POOLS  " +
                                    "WHERE ID = ? ",
                                    new Object [] { Integer.valueOf ( extensionPoolID )  } );
        }
        catch ( RemoteException e) {
            logFatal ( e.toString() );

            throw new EJBException(
                collateErrorMessages(   "UC622",
                                        "E4080",
                                        new Object[]{ extensionPoolID } ) );
        }
        catch ( Exception e) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC622",
                                        "E2033",
                                        new Object[]{ extensionPoolID } ),
                e );
        }
    }


    /**
     * editExtensionPool allows administrators to change attributes for
     * a given extension pool (currently just the name).
     *
     * @param extensionPoolID the PK of the extension pool that you want
     * to modify
     * @param name the name that you want to change the pool to.
     * @throws com.pingtel.pds.common.PDSException is thrown for application level errors.
     */
    public void editExtensionPool ( String extensionPoolID, String name )
            throws PDSException {

        try {
            if (name != null && name.trim().length() == 0) {
                mCTX.setRollbackOnly();

                throw new PDSException(collateErrorMessages("E3030", new Object[]{"name"}));
            }
            if ( name != null && name.length() > MAX_NAME_LEN ) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "E3028",
                                            new Object[]{
                                                "name",
                                                new Integer(MAX_NAME_LEN)}));
            }

            checkExtensionPoolNameUnique(name);

            executePreparedUpdate ( "UPDATE EXTENSION_POOLS " +
                                    "SET NAME = ? " +
                                    "WHERE ID = ?",
                                    new Object [] { name,
                                                    Integer.valueOf ( extensionPoolID ) } );
        }
        catch (SQLException e) {
            mCTX.setRollbackOnly();

            /** TODO */
            throw new PDSException( e.toString());
        }
    }

    /**
     * getNextExtension returns the next available extension number for a given
     * pool.
     *
     * @param extensionPoolID the extension pool ID for the pool that you want
     * to allocate the extension from.
     * @return next available extension.
     * @throws com.pingtel.pds.common.PDSException for application errors.
     */
    public String getNextExtension ( String extensionPoolID ) throws PDSException {

        String nextExtension = null;

        try {
            /////////////////////////////////////////////////////////////////
            //
            // The following SQL statement looks pretty bizaar.   It basically
            // finds the next available extension for the given pool.   The
            // whole TO_NUMBER bit is to convert the varchar extension_number
            // values into numbers.   Postgres requires that you give a
            // format mask for this function (Oracle does not).   FYI to_number
            // is an Oracle function that Postgres copied not ANSI.
            //
            /////////////////////////////////////////////////////////////////
            ArrayList rows;
            rows = executePreparedQuery (
                "SELECT MIN( TO_NUMBER( EXTENSION_NUMBER, '999999999999999999999999999999' ) ) " +
                "FROM EXTENSIONS, EXTENSION_POOLS " +
                "WHERE EXTENSION_POOLS.ID = ? " +
                "AND EXTENSION_POOLS.ID = EXTENSIONS.EXT_POOL_ID " +
                "AND EXTENSIONS.STATUS='A'",
                new Object [] { Integer.valueOf( extensionPoolID ) },
                1,
                1 );

            for ( Iterator iRows = rows.iterator(); iRows.hasNext(); ) {
                ArrayList row = (ArrayList) iRows.next();

                nextExtension = (String) row.get( 0 );
            }
        }
        catch ( SQLException e) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "E5020",
                                        new Object[]{ extensionPoolID } ),
                e );
        }

        return nextExtension;
    }


    /**
     * allocateExtensionsToPool adds a contiguous range or integer extensions to
     * a given extension pool.
     *
     * @param extensionPoolID the ID/PK of the extension pool that the extensions
     * should be added to.
     * @param minExtension the lower end value for the extensions.
     * @param maxExtension the upper end value for the extensions.
     * @throws com.pingtel.pds.common.PDSException application errors.
     */
    public void allocateExtensionsToPool (  String extensionPoolID,
                                            String minExtension,
                                            String maxExtension ) throws PDSException {

        int min;
        int max;

        try {
            min = new Integer ( minExtension ).intValue();
        }
        catch (NumberFormatException e) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC623",
                                        "E3021",
                                        new Object[]{ minExtension } ),
                e );
        }

        try {
            max = new Integer ( maxExtension ).intValue();
        }
        catch (NumberFormatException e) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC623",
                                        "E3021",
                                        new Object[]{ maxExtension } ),
                e );
        }

        for ( int counter = min; counter <= max; ++counter) {
            try {

                // This checks to see that the extension isn't owned by an existing
                // extension pool.
                ArrayList rows = executePreparedQuery ( "SELECT EXTENSION_NUMBER " +
                                                        "FROM EXTENSIONS " +
                                                        "WHERE EXTENSION_NUMBER = ? ",
                                                        new Object [] { String.valueOf ( counter ) },
                                                        1,
                                                        1 );

                if ( rows.isEmpty() ) {

                    // Now check to see if a user has already has the extension -
                    // this can happen when a user enters an extension which is
                    // not currently in any pool.

                    Collection existingUsers =
                            mUserHome.findByDisplayIDOrExtension( String.valueOf( counter ) );

                    if ( existingUsers.isEmpty() ) {
                        executePreparedUpdate (
                                "INSERT INTO EXTENSIONS ( EXTENSION_NUMBER, EXT_POOL_ID,STATUS ) " +
                                "VALUES ( ?, ?, 'A' ) ",
                                new Object []{ new Integer ( counter ), extensionPoolID } );
                    }
                    else {
                        executePreparedUpdate (
                                "INSERT INTO EXTENSIONS ( EXTENSION_NUMBER, EXT_POOL_ID,STATUS ) " +
                                "VALUES ( ?, ?, 'U' ) ",
                                new Object []{ new Integer ( counter ), extensionPoolID } );
                    }
                }
            }
            catch ( SQLException e) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                collateErrorMessages(   "UC623",
                                        "E5021",
                                        new Object[]{ extensionPoolID } ),
                e );
            }
            catch (FinderException e) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                collateErrorMessages(   "UC623",
                                        "E5021",
                                        new Object[]{ extensionPoolID } ),
                e );
            }
            catch (RemoteException e) {
                logFatal ( e.toString() );

                throw new EJBException(
                    collateErrorMessages(   "UC623",
                                            "E5021",
                                            new Object[]{ extensionPoolID } )
                );
            }
        }
    }


    /**
     * deleteExtensionRange deletes the given extensions from the system.  This
     * can only be done if the extensions are not currently assigned to a
     * user if one is then a PDSException is thrown.   Extensions within
     * the range are only deleted if they belong to the extenions pool
     * whose ID is given.
     *
     * @param extensionPoolID is the extension pool's PK that you want to delete the
     * extensions from.
     * @param minExtension the min extension that you want to delete.
     * @param maxExtension the max extension that you want to delete.
     * @throws com.pingtel.pds.common.PDSException for application errors.
     */
    public void deleteExtensionRange (  String extensionPoolID,
                                        String minExtension,
                                        String maxExtension) throws PDSException {

        try {
            Integer intMinExtension = null;
            Integer intMaxExtension = null;

            try {
                intMinExtension = new Integer ( minExtension );
            }
            catch (NumberFormatException e) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC624",
                                            "E5020",
                                            new Object[]{ minExtension } ),
                    e );
            }

            try {
                intMaxExtension = new Integer ( maxExtension );
            }
            catch (NumberFormatException e) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC624",
                                            "E5020",
                                            new Object[]{ maxExtension } ),
                    e );
            }


            for (   int count = intMinExtension.intValue();
                    count <= intMaxExtension.intValue();
                    ++count ) {

                Collection allocatedUsers = mUserHome.findByExtension( String.valueOf( count ) );

                for ( Iterator iUser = allocatedUsers.iterator(); iUser.hasNext(); ) {
                    User user = (User) iUser.next();

                    user.setExtension( null );
                }
            }

            try {
                executePreparedUpdate ( "DELETE FROM EXTENSIONS " +
                                        "WHERE TO_NUMBER ( EXTENSION_NUMBER, '999999999999999999999999999999' ) >= TO_NUMBER ( ?, '999999999999999999999999999999' ) " +
                                        "AND TO_NUMBER ( EXTENSION_NUMBER, '999999999999999999999999999999' ) <= TO_NUMBER ( ?, '999999999999999999999999999999' ) " +
                                        "AND EXT_POOL_ID = ?",
                                        new Object [] { minExtension,
                                                        maxExtension,
                                                        extensionPoolID } );
            }
            catch (SQLException e) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC624",
                                            "E6024",
                                            new Object[]{ minExtension } ),
                    e );
            }

        }
        catch ( FinderException e) {
            logFatal( e.toString() );

            throw new EJBException(
                    collateErrorMessages(   "UC624",
                                            "E4081",
                                            new Object[]{ minExtension } ),
                    e );
        }
        catch ( RemoteException e) {
            mCTX.setRollbackOnly();

            throw new EJBException(
                    collateErrorMessages(   "UC624",
                                            "E4081",
                                            new Object[]{ minExtension } ),
                    e );
        }
    }


    /**
     * getAllExtensionPoolsDetails returns a Map of ID to Name pairs for all
     * of the extension pools in the system.
     * @return Map of ID to Name pairs
     * @throws com.pingtel.pds.common.PDSException for applicatione errors.
     */
    public Map getAllExtensionPoolsDetails () throws PDSException {

        HashMap returnValues = new HashMap();

        try {
            ArrayList rows;

            rows = executePreparedQuery (   "SELECT ID, NAME " +
                                            "FROM EXTENSION_POOLS ",
                                            null,
                                            2,
                                            1000000 );

            for ( Iterator iRows = rows.iterator(); iRows.hasNext(); ) {
                ArrayList row = (ArrayList) iRows.next();

                String id = (String) row.get( 0 );
                String name = (String) row.get( 0 );
                returnValues.put( id, name );
            }
        }
        catch ( SQLException e) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "E8007",
                                        null ),
                e );
        }

        return returnValues;
    }


    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////
    private void checkExtensionPoolNameUnique(String name) throws SQLException, PDSException {
        // check for uniqueness of the extension pool name
        ArrayList rows =
                executePreparedQuery(
                        "SELECT NAME FROM EXTENSION_POOLS WHERE NAME =?",
                        new Object [] {name},
                        1,
                        1);

        if (!rows.isEmpty()) {
            mCTX.setRollbackOnly();

            throw new PDSException(collateErrorMessages("E3031", new Object[]{name}));
        }
    }



    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}
