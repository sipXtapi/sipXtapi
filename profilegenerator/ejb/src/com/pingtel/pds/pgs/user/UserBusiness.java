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


import java.rmi.RemoteException;

import javax.ejb.EJBObject;

import com.pingtel.pds.common.PDSException;


public interface UserBusiness {

    public static final int MAX_DISPLAY_ID_LEN = 80;
    public static final int MAX_FIRST_NAME_LEN = 20;
    public static final int MAX_LAST_NAME_LEN = 30;
    public static final int MAX_EXTENSION_LEN = 30;
    public static final int MAX_ALIAS_LEN = 256;

    /**
     * standard EJ Bean implementation
     *
     *@return  The organizationID value
     */
    Integer getOrganizationID() throws RemoteException;

    /**
     * Sets the organizationID attribute of the UserBean object
     *
     * @param organizationID PK of the Organization that this
     * user it to be assigned to.
     */
    void setOrganizationID(Integer organizationID) throws RemoteException;

    /**
     * returns the firstName attribute of the User
     *
     * @return firstName of this User
     */
    String getFirstName() throws RemoteException;

    /**
     * Sets the firstName attribute of the User
     *
     * @param firstName the new firstName for this User
     */
    void setFirstName(String firstName) throws PDSException, RemoteException;

    /**
     * returns the lastName attribute of the User
     *
     * @return the lastName of this User
     */
    String getLastName() throws RemoteException;

    /**
     * sets the lastName of a User
     *
     * @param lastName the new lastName for this User
     */
    void setLastName(String lastName) throws PDSException, RemoteException;

    /**
     * returns the PK of the UserGroup that is User belongs to.
     *
     * @return the PK of the owning UserGroup
     */
    Integer getUserGroupID() throws RemoteException;

    /**
     * assigns this User to a UserGroup.
     *
     * @param userGroupID the PK of the UserGroup that this User is to
     * belong to.
     */
    void setUserGroupID(Integer userGroupID) throws RemoteException, PDSException;

    /**
     * returns the PK of the RefConfigurationSet that is currently
     * assigned to this User.
     *
     * @return the PK of the RefConfigurationSet assigned to this
     * User
     */
    Integer getRefConfigSetID() throws RemoteException;

    /**
     * assigns a RefConfigurationSet to the User.
     *
      * @param refConfigSetID K of the RefConfigurationSet to be
     *  assigned to this User.
     */
    void setRefConfigSetID(Integer refConfigSetID) throws RemoteException;

    /**
     * returns the password or 'PIN' for this User.   The password
     * is encrypted elsewhere in the code.
     *
     * @return the password or 'PIN' for this User
     */
    String getPassword() throws RemoteException;

    /**
     * Sets the password attribute of the User.
     *
     * @param password the new password value
     */
    void setPassword(String password) throws PDSException, RemoteException;

    /**
     * returns the id attribute for this User.  Id refers to the
     * 'hidden' auto-generated PK for this User.   To the users
     * of Config Server Users' Ids are really the displayId.
     *
      * @return the id (PK) of this User.
     */
    String getID() throws RemoteException;

    /**
     * returns the displayID attribute of the User
     *
     * @return The display id of this User
     */
    String getDisplayID() throws RemoteException;

    /**
     * returns this Users extension (if one exists).
     *
     * @return this Users extension (if one exists).
     */
    String getExtension () throws RemoteException;

    /**
     * sets this Users extension.   This is quite and involved method
     * as it has to perform a number of validation checks and also
     * has to mark the extension value as being 'unavailable' if it
     * exists in an extension pool.
     *
     * @param extension new extension to be assigned to this User.
     * @throws PDSException if the extension value is either invalid
     * or is being used elsewhere as another User's Id or extension.
     * @see ExtensionPoolAdvocateBusiness
     */
    void setExtension (String extension) throws PDSException, RemoteException;

    /**
     * returns a comma-separated list of this Users aliases.
     * A User may have any number of aliases.
     *
     * @return comma-separated list of this Users aliases
     */
    String getAliases () throws RemoteException;

    /**
     * set the aliases for the User.  This is sort-of clunky as
     * you have to pass in all of the aliases, there is no attempt to
     * reconcile partial lists.
     *
     * @param aliases a comma-separated list of aliases to be assigned
     * to this User.
     */
    void setAliases ( String aliases ) throws RemoteException;

    /**
     * setDisplayID changes the displayId attribute for this User.
     * This is quite and involved method as it has to perform a number
     * of validation checks and also has to mark the displayID value
     * as being 'unavailable' if it exists in an extension pool.
     *
     * @param displayID new displayID to be assigned to this User.
     * @throws PDSException if the displayID value is either invalid
     * or is being used elsewhere as another User's Id or extension.
     * @see ExtensionPoolAdvocateBusiness
     * @param displayID
     * @throws PDSException
     */
    void setDisplayID(String displayID) throws PDSException, RemoteException;

    /**
     * getExternalID returns a more user-friendly description of this User.
     * This is used primarly for logging and auditing purposes.
     *
     * @return a user-friendly description of this User
     * @throws PDSException is thrown for all errors.
     */
    String getExternalID () throws PDSException, RemoteException;

    /**
     * calculatePrimaryLineURLgets you the Primary Line URL for a given user.
     *
     * @return the SIP URL for this User's Primary Line.
     * @throws PDSException is thrown for all non-system errors.
     */
    String calculatePrimaryLineURL() throws PDSException, RemoteException;

    /**
     * returns true if this User owns any devices.  This is an optimization
     * to short cut the slow finder way of doing this.
     *
     * @return true if the User has >0 Devices assigned to them.
     */
    boolean userHasDevices () throws PDSException, RemoteException;

    /**
     * getProfileEncryptionKey returns this Users profileEncryptionKey.
     *
     * @return this Users profileEncryptionKey
     */
    String getProfileEncryptionKey() throws RemoteException;

    /**
     * setProfileEncryptionKey allows you to change this Users
     * profileEncryptionKey.
     *
     * @param profileEncryptionKey new value to use for the
     * Users profileEncryptionKey.
     */
    void setProfileEncryptionKey(String profileEncryptionKey) throws RemoteException;


}