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


package com.pingtel.pds.pgs.user;

import javax.ejb.*;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import java.rmi.RemoteException;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.StringTokenizer;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationHome;


/**
 * UserBean is the EJ Bean implementation class for the User entity.
 *
 * @author ibutcher
 *
 */
public class UserBean extends JDBCAwareEJB implements EntityBean, UserBusiness {

//////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final String PRINCIPALS_QUERY =
        "   SELECT id " +
        "   FROM users " +
        "   WHERE display_id = ? " +
        "   OR extension = ?";

//////////////////////////////////////////////////////////////////////////
// Attributes
////
    // Home references
    private OrganizationHome mOrganizationHome;

    // Clearing Entity Bean references
    private Organization mOrganizationEJBObject;

    // Bean Context
    private EntityContext mCtx;

    // bean attributes
    public Integer organizationID;
    public String firstName;
    public String lastName;
    public Integer userGroupID;
    public Integer refConfigSetID;
    public String password;
    public String displayID;
    public String id;
    public String extension;

    // profileEncryptionKey is the value that is used as the key to
    // encrypt the Users profiles.
    public String profileEncryptionKey;


//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public String ejbCreate(    Integer organizationID,
                                String firstName,
                                String lastName,
                                String password,
                                String displayID,
                                Integer refConfigSetID,
                                Integer userGroupID,
                                String extension )
        throws CreateException, PDSException {

        // note: the order of these method call IS significant
        this.id = String.valueOf( getNextSequenceValue("USER_SEQ") );
        setOrganizationID (organizationID);
        setFirstName(firstName);
        setLastName(lastName);
        setPassword(password);
        setDisplayID (displayID);
        setRefConfigSetID(refConfigSetID);
        setUserGroupID(userGroupID);

        if ( extension != null && extension.trim().length() == 0 ) {
            setExtension ( null );
        }
        else {
            setExtension ( extension );
        }

        return null;
    }


    public void ejbPostCreate(  Integer organizationID,
                                String firstName,
                                String lastName,
                                String password,
                                String displayID,
                                Integer refConfigSetID,
                                Integer userGroupID,
                                String extension ) throws PDSException {

        if ( getExtension() != null ) {
            if ( getExtension().equalsIgnoreCase( getDisplayID() ) ) {
                mCtx.setRollbackOnly();
                throw new PDSException ( "Users can not have identical IDs and extensions" );
            }
        }
    }


    /**
     *  standard EJ Bean implementation
     */
    public void ejbLoad() { }


    /**
     *  standard EJ Bean implementation
     */
    public void ejbStore() { }


    /**
     *  standard EJ Bean implementation
     */
    public void ejbRemove() throws RemoveException {
        try {
            deleteAliases();
            updateExtensionStatus ( extension, "A" );
            updateExtensionStatus ( displayID, "A" );
        }
        catch ( SQLException ex ) {
            throw new RemoveException ( ex.toString() );
        }
        catch (com.pingtel.pds.common.PDSException e) {
            throw new RemoveException ( e.toString() );
        }

    }


    /**
     *  standard EJ Bean implementation
     */
    public void ejbActivate() {
        mOrganizationEJBObject = null;
    }


    /**
     *  standard EJ Bean implementation
     */
    public void ejbPassivate() { }


    /**
     *  standard EJ Bean implementation
     *
     * @param  ctx  The new entityContext value
     */
    public void setEntityContext(EntityContext ctx) {
        this.mCtx = ctx;

        try {
            Context initial = new InitialContext();
            mOrganizationHome = (OrganizationHome) initial.lookup ("Organization" );
        }
        catch (NamingException ne) {
            logFatal( ne.toString(), ne );
            throw new EJBException(ne);
        }
    }


    /**
     *  standard EJ Bean implementation
     */
    public void unsetEntityContext() {
        this.mCtx = null;
    }


    /**
     * standard EJ Bean implementation
     *
     *@return  The organizationID value
     */
    public Integer getOrganizationID() {
        return organizationID;
    }


    /**
     * Sets the organizationID attribute of the UserBean object
     *
     * @param organizationID PK of the Organization that this
     * user it to be assigned to.
     */
    public void setOrganizationID(Integer organizationID) {
        this.organizationID = organizationID;
    }


    /**
     * returns the firstName attribute of the User
     *
     * @return firstName of this User
     */
    public String getFirstName() {
        return firstName;
    }


    /**
     * Sets the firstName attribute of the User
     *
     * @param firstName the new firstName for this User
     */
    public void setFirstName(String firstName) throws PDSException {
        if (firstName != null && firstName.length() > MAX_FIRST_NAME_LEN ) {
            mCtx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "E3025",
                                        new Object[]{
                                            new Integer(MAX_FIRST_NAME_LEN)}));
        }
        this.firstName = firstName;
    }


    /**
     * returns the lastName attribute of the User
     *
     * @return the lastName of this User
     */
    public String getLastName() {
        return lastName;
    }


    /**
     * sets the lastName of a User
     *
     * @param lastName the new lastName for this User
     */
    public void setLastName(String lastName) throws PDSException {
        if (lastName != null && lastName.length() > MAX_LAST_NAME_LEN ) {
            mCtx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "E3026",
                                        new Object[]{
                                            new Integer(MAX_LAST_NAME_LEN)}));
        }
        this.lastName = lastName;
    }


    /**
     * returns the PK of the UserGroup that is User belongs to.
     *
     * @return the PK of the owning UserGroup
     */
    public Integer getUserGroupID() {
        return userGroupID;
    }

    /**
     * assigns this User to a UserGroup.
     *
     * @param userGroupID the PK of the UserGroup that this User is to
     * belong to.
     */
    public void setUserGroupID(Integer userGroupID) throws PDSException {
        if (userGroupID == null) {
            if (!(displayID.equalsIgnoreCase("superadmin") || displayID.equalsIgnoreCase("SDS"))){
                mCtx.setRollbackOnly();
                throw new PDSException(collateErrorMessages("E3040", null));
            }
        }
        this.userGroupID = userGroupID;
    }

    /**
     * returns the PK of the RefConfigurationSet that is currently
     * assigned to this User.
     *
     * @return the PK of the RefConfigurationSet assigned to this
     * User
     */
    public Integer getRefConfigSetID() {
        return this.refConfigSetID;
    }

    /**
     * assigns a RefConfigurationSet to the User.
     *
      * @param refConfigSetID K of the RefConfigurationSet to be
     *  assigned to this User.
     */
    public void setRefConfigSetID(Integer refConfigSetID) {
        this.refConfigSetID = refConfigSetID;
    }

    /**
     * returns the password or 'PIN' for this User.   The password
     * is encrypted elsewhere in the code.
     *
     * @return the password or 'PIN' for this User
     */
    public String getPassword() {
        return password;
    }


    /**
     * Sets the password attribute of the User.
     *
     * @param password the new password value
     */
    public void setPassword(String password) throws PDSException {

        if ( password == null )
            password = "";

        this.password = password;
    }

    /**
     * returns the id attribute for this User.  Id refers to the
     * 'hidden' auto-generated PK for this User.   To the users
     * of Config Server Users' Ids are really the displayId.
     *
      * @return the id (PK) of this User.
     */
    public String getID() {
        return this.id;
    }

    /**
     * returns the displayID attribute of the User
     *
     * @return The display id of this User
     */
    public String getDisplayID() {
        return this.displayID;
    }

    /**
     * returns this Users extension (if one exists).
     *
     * @return this Users extension (if one exists).
     */
    public String getExtension () {
        return this.extension;
    }


    /**
     * sets this Users extension.   This is quite and involved method
     * as it has to perform a number of validation checks and also
     * has to mark the extension value as being 'unavailable' if it
     * exists in an extension pool.
     *
     * @param extension new extension to be assigned to this User.
     * @throws PDSException if the extension value is either invalid
     * or is being used elsewhere as another User's Id or extension.
     * @see com.pingtel.pds.pgs.user.ExtensionPoolAdvocateBusiness
     */
    public void setExtension (String extension) throws PDSException {

        if ( extension != null && extension.trim().length() == 0 ) {
            extension = null;
        }

        if( extension != null ) {
            if ( extension.length() > MAX_EXTENSION_LEN ) {
                mCtx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "E3027",
                                            new Object[]{
                                                new Integer(MAX_EXTENSION_LEN)}));
            }

            char [] extensionChars = extension.toCharArray();

            for (int counter = 0; counter < extensionChars.length; ++counter) {
                if ( extensionChars[counter] < '0' || extensionChars[counter] > '9') {
                    mCtx.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages (  "E3020",
                                                new Object []{  getDisplayID(),
                                                                extension } ) );
                }
            }

            if (!checkIDOrExtensionUnique ( extension )){
                mCtx.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "E3033",
                                                new Object[]{extension}) );
            }

            updateExtensionStatus ( extension, "U" );

            try {
                if ( inExtensionPool ( extension, "reserved" ) ) {
                    mCtx.setRollbackOnly();

                    throw new PDSException(
                            collateErrorMessages(   "E3032",
                                                    new Object[]{ "Extension", extension }) );
                }
            }
            catch (SQLException e) {
                throw new EJBException ( e.getMessage() );
            }
        }

        updateExtensionStatus ( this.extension, "A" );

        this.extension = extension;
    }


    /**
     * returns a comma-separated list of this Users aliases.
     * A User may have any number of aliases.
     *
     * @return comma-separated list of this Users aliases
     */
    public String getAliases () {

        StringBuffer aliases = new StringBuffer();

        try {
            ArrayList rows =
                this.executePreparedQuery(  "   SELECT      alias " +
                                            "   FROM        aliases " +
                                            "   WHERE       user_id = ? " +
                                            "   ORDER BY    alias_order " ,
                                            new Object [] { id },
                                            1,
                                            100000 );

            for ( Iterator iRow = rows.iterator(); iRow.hasNext(); ) {
                ArrayList row = (ArrayList) iRow.next();
                aliases.append( (String) row.get( 0 ));
                if ( iRow.hasNext() )
                    aliases.append( "," );
            }

        }
        catch ( SQLException ex ) {
            ex.printStackTrace();
        }

        return aliases.toString();
    }


    /**
     * set the aliases for the User.  This is sort-of clunky as
     * you have to pass in all of the aliases, there is no attempt to
     * reconcile partial lists.
     *
     * @param aliases a comma-separated list of aliases to be assigned
     * to this User.
     */
    public void setAliases ( String aliases ) {

        try {
            deleteAliases();

            if ( aliases != null && aliases.length() != 0 ) {
                StringTokenizer tok = new StringTokenizer ( aliases, "," );
                int counter = 1;

                while ( tok.hasMoreTokens() ) {
                    String alias = tok.nextToken();

                    executePreparedUpdate ( "   INSERT INTO aliases ( user_id, alias, alias_order ) " +
                                            "   VALUES ( ?,?,? ) ",
                                            new Object [] { this.id, alias.trim(), new Integer ( counter++ ) } );
                }
            }
        }
        catch ( SQLException ex ) {
            throw new EJBException ( ex.toString() );
        }
    }

    /**
     * setDisplayID changes the displayId attribute for this User.
     * This is quite and involved method as it has to perform a number
     * of validation checks and also has to mark the displayID value
     * as being 'unavailable' if it exists in an extension pool.
     *
     * @param displayID new displayID to be assigned to this User.
     * @throws PDSException if the displayID value is either invalid
     * or is being used elsewhere as another User's Id or extension.
     * @see com.pingtel.pds.pgs.user.ExtensionPoolAdvocateBusiness
     * @param displayID
     * @throws PDSException
     */
    public void setDisplayID(String displayID) throws PDSException {

        // displayId can not be null
        if (displayID == null){
            mCtx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("E3039", null));
        }

        if (displayID != null && displayID.length() > MAX_DISPLAY_ID_LEN) {
            mCtx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "E3024",
                                        new Object[]{
                                            new Integer(MAX_DISPLAY_ID_LEN)}));
        }

        // OK this is a bit bizaar.   User IDs (aka displayIDs) and extensions
        // come from the same namespace.   This means that you may have an
        // extension record for an extension pool which is marked as unavailable
        // because that ID/number belongs a user.  Therefore when you change
        // the displayID you have to free-up the extension record so that it
        // becomes allocatable to it's pool.
        updateExtensionStatus ( this.displayID, "A" );

        if(!checkIDOrExtensionUnique ( displayID )) {
            mCtx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages(   "E3034",
                                            new Object[]{displayID}) );
        }

        try {
            if ( inExtensionPool ( displayID, "reserved" )  ) {
                    mCtx.setRollbackOnly();

                    throw new PDSException(
                            collateErrorMessages(   "E3032",
                                                    new Object[]{ "User ID", displayID }) );
            }
        }
        catch (SQLException e) {
            throw new EJBException ( e.getMessage() );
        }

        this.displayID = displayID;
    }


    /**
     * getProfileEncryptionKey returns this Users profileEncryptionKey.
     *
     * @return this Users profileEncryptionKey
     */
    public String getProfileEncryptionKey() {
        return profileEncryptionKey;
    }

    /**
     * setProfileEncryptionKey allows you to change this Users
     * profileEncryptionKey.
     *
     * @param profileEncryptionKey new value to use for the
     * Users profileEncryptionKey.
     */
    public void setProfileEncryptionKey(String profileEncryptionKey) {
        this.profileEncryptionKey = profileEncryptionKey;
    }



    /**
     * getExternalID returns a more user-friendly description of this User.
     * This is used primarly for logging and auditing purposes.
     *
     * @return a user-friendly description of this User
     * @throws PDSException is thrown for all errors.
     */
    public String getExternalID () throws PDSException{
        Organization organization = getMyOrganization();
        try {
            return "id: " + this.displayID +
                " organization: " + organization.getExternalID();
        }
        catch ( Exception ex ) {
            logFatal( ex.toString(), ex );
            throw new EJBException ( ex );
        }
    }



    /**
     * calculatePrimaryLineURLgets you the Primary Line URL for a given user.
     *
     * @return the SIP URL for this User's Primary Line.
     * @throws PDSException is thrown for all non-system errors.
     */
    public String calculatePrimaryLineURL() throws PDSException {

        try {
            Organization org = getMyOrganization();

            StringBuffer xmlContent = new StringBuffer();

            boolean needsWrapping = false;

            if ( getFirstName() != null && getFirstName().length() > 0 ) {
                xmlContent.append( getFirstName() );
                needsWrapping = true;
            }

            if ( getLastName() != null && getLastName().length() > 0 ) {
                if ( needsWrapping ) {
                    xmlContent.append( " " );
                }

                xmlContent.append( getLastName() );
                needsWrapping = true;
            }

            if ( needsWrapping ) {
                xmlContent.append( "<" );
            }

            xmlContent.append( "sip:" );
            xmlContent.append( getDisplayID() );
            xmlContent.append( "@" + org.getDNSDomain() );

            if ( needsWrapping ) {
                xmlContent.append( ">" );
            }

            return xmlContent.toString();
        }
        catch ( RemoteException ex ) {
            throw new EJBException ( ex.toString() );
        }
    }


    /**
     * returns true if this User owns any devices.  This is an optimization
     * to short cut the slow finder way of doing this.
     *
     * @return true if the User has >0 Devices assigned to them.
     */
    public boolean userHasDevices () throws PDSException {

        boolean returnValue = false;

        try {
            int count = -1;

            ArrayList rows =
                    executePreparedQuery(
                            "SELECT COUNT(*) FROM LOGICAL_PHONES WHERE USRS_ID = ?",
                            new Object [] { getID() },
                            1,
                            1 );

            for ( Iterator iRow = rows.iterator(); iRow.hasNext(); ) {
                ArrayList row = (ArrayList) iRow.next();
                String countString = (String) row.get(0);
                count = Integer.valueOf( countString ).intValue();
            }

            if ( count > 0 ) {
                returnValue = true;
            }
        }
        catch ( SQLException ex ) {
            throw new EJBException(ex.getMessage());
        }

        return returnValue;
    }




//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    private void updateExtensionStatus ( String extensionNumber, String status)
            throws PDSException {

        try {
            if ( extensionNumber != null ) {
                executePreparedUpdate ( "UPDATE EXTENSIONS " +
                                        "SET STATUS = ? " +
                                        "WHERE EXTENSION_NUMBER = ? ",
                                        new Object [] { status, extensionNumber } );
            }
        }
        catch ( SQLException e) {
            mCtx.setRollbackOnly();

            throw new PDSException (
                collateErrorMessages (  "E5019",
                                        new Object [] { this.displayID, extensionNumber } ) );
        }
    }


    private void deleteAliases () throws SQLException {

        executePreparedUpdate ( "   DELETE FROM aliases " +
                                "   WHERE user_id = ? ",
                                    new Object [] { this.id } );

    }


    private Organization getMyOrganization() throws PDSException {
        if ( mOrganizationEJBObject == null ) {

            try {
                mOrganizationEJBObject =
                    mOrganizationHome.findByPrimaryKey( getOrganizationID() );
            }
            catch ( FinderException ex ) {
                mCtx.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "E1018",
                                                new Object[]{ getOrganizationID() }),
                        ex);
            }
            catch ( RemoteException ex ) {
                logFatal ( ex.toString(), ex );

                throw new EJBException( ex.toString());
            }
        }

        return mOrganizationEJBObject;
    }


    private boolean checkIDOrExtensionUnique ( String id ) {
        boolean isUnique = false;

        if ( id != null ) {
            try {
                ArrayList rows =
                    executePreparedQuery(   PRINCIPALS_QUERY,
                                            new Object [] { id, id },
                                            1,
                                            100000 );

                for(Iterator iRows = rows.iterator(); iRows.hasNext(); ) {
                    ArrayList row = (ArrayList) iRows.next();
                    String queryID = (String) row.get(0);
                    if (getID().equals(queryID)){
                        isUnique = true;
                    }
                }

                if (rows.isEmpty()){
                    isUnique = true;
                }
            }
            catch (SQLException e) {
                logFatal(e.getMessage(), e);
                throw new EJBException(e.getMessage() );
            }
        }

        return isUnique;
    }

    private boolean inExtensionPool ( String extension, String poolName )
            throws SQLException {

        ArrayList rows;

        rows = executePreparedQuery (   "select extension_number " +
                                        "from extension_pools, extensions " +
                                        "where extension_pools.id = extensions.ext_pool_id " +
                                        "and extension_pools.name = ? " +
                                        "and extension_number = ?",
                                        new Object [] { poolName, extension },
                                        1,
                                        1 );

        return !rows.isEmpty();
    }



//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
