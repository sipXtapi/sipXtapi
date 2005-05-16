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

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.phone.Device;

import java.rmi.RemoteException;
import java.util.Collection;

public interface UserAdvocateBusiness {

    /**
     *  createUser is the session facade for the "Create User" use case.
     *
     * @param firstName is the first name of the user to be created
     * @param lastName is the last name of the user to be created
     * @param password is the new users password - used for authentication
     * @param id is the unique user ID (user visible) for the new user.
     * @param refConfigSetID is the primary key of the Reference
     * Configuration Set that the User is assigned.
     * @param organizationID The internal ID of the organization that this user
     * belongs to
     * @param userGroupID the internal ID of the user group that this user
     * belongs to.
     * @param extension extension to be assigned to the user.
     * @return EJBObject of the new user.
     * @exception PDSException is thrown for non-fatal application exceptions
     */
    User createUser(    String organizationID,
                        String firstName,
                        String lastName,
                        String password,
                        String id,
                        String refConfigSetID,
                        String userGroupID,
                        String extension ) throws PDSException, RemoteException;

    /**
     * This version of createUser is the session facade for the
     * "Create User" use case which is used 'locally' i.e. between
     * EJ Beans (not the web UI).
     *
     * @param firstName is the first name of the user to be created
     * @param lastName is the last name of the user to be created
     * @param password is the new users password - used for authentication
     * @param id is the unique user ID (user visible) for the new user.
     * @param refConfigSetID is the primary key of the Reference
     * Configuration Set that the User is assigned.
     * @param organizationID The internal ID of the organization that this user
     * belongs to
     * @param userGroupID the internal ID of the user group that this user
     * belongs to.
     * @param extension extension to be assigned to the user.
     * @return EJBObject of the new user.
     * @exception PDSException is thrown for non-fatal application exceptions
     */
    User createUser( Integer organizationID,
                            String firstName,
                            String lastName,
                            String password,
                            String id,
                            Integer refConfigSetID,
                            Integer userGroupID,
                            String extension ) throws PDSException, RemoteException;

    /**
     * deleteUser implements the "delete user" use case.  It deletes
     * the user entity and all the user's dependent information
     * (configuration set, roles).  It doesn't delete any devices
     * that the user currently owns.
     *
     * @param userID the PK of the user to be deleted
     * @throws PDSException is thrown for application level errors.
     */
    void deleteUser(String userID) throws PDSException, RemoteException;

    
    /**
     * editUser is the session facade for the "Edit User" use case.  If null
     * values are supplied for parameters then the existing values are kept.
     * The exception being the 'id' parameter as this is used to identify the
     * user.
     *
     * @param id is the unique user ID for the user to be modified
     * @param displayID the ID that is displayed to users in the UI.
     * @param firstName the value to substitute for the existing
     * firstName if this is null then no change is made.
     * @param lastName the value to substitute for the existing
     * lastName if this is null then no change is made.
     * @param password the value to substitute for the existing
     * password if this is null then no change is made.
     * @param refConfigSetID the value to substitute for the existing
     * reference configuration set ID if this is null then no change is made.
     * @param userGroupID the user group ID that you wish to assign to the user.
     * @param aliases users may have >0 aliases which are used by the comm.
     * server if present.   These are supplied in a comma separated list.
     * @param extension a user may have a numeric extension
     * @exception PDSException is thrown for non-fatal application exceptions
     */
    boolean [] editUser( String id,
                                String displayID,
                                String firstName,
                                String lastName,
                                String password,
                                String refConfigSetID,
                                String userGroupID,
                                String aliases,
                                String extension ) throws PDSException, RemoteException;

    /**
     * changes the given Users password.
     *
     * @param user User whose password is to be set
     * @param displayID the display ID or externally visible ID for the User
     * @param password the plain-text password to be used
     * @throws PDSException is thrown for application level errors
     */
    void setUsersSipPassword( User user, String displayID, String password )
            throws PDSException, RemoteException;

    /**
     * changes the given Users password.
     *
     * @param id PK of the User whose password is to be set
     * @param displayID the display ID or externally visible ID for the User
     * @param password the plain-text password to be used
     * @throws PDSException is thrown for application level errors
     */
    void setUsersSipPassword( String id, String displayID, String password )
            throws PDSException, RemoteException;
    
    void setUsersPinToken( String id, String displayID, String password )
            throws PDSException, RemoteException;

    void setUsersPinToken( User user, String displayID, String password )
        throws PDSException, RemoteException;

    /**
     * fixPrimaryLine modifies previously generated primary line property settings.   This
     * now includes the Pingtel 'PRIMARY_LINE' settings and the Cisco 79xx 'line1' setting.
     * The undigested password is needed as we can't determine the plain-text password that
     * the user originally entered from the digested value that we store.
     *
     * @param user User whose line definitions need modifying
     * @param fixCredential do we need to reset the credential values for the Pingtel primary
     * line setting
     * @param undigestedPassword see method description.  Plain text password
     * @throws PDSException is thrown for application level errors
     */
    void fixPrimaryLine ( User user, boolean fixCredential, String undigestedPassword )
         throws PDSException, RemoteException;
    
    /**
     * fixDnsDomain modifies previously generated primary line property settings
     * It replaces original DNS name with a new name.
     * It needs to be called every time organization DNS name changes
     * 
     * @param organization
     * @param oldDomainName previous domain name value
     * @throws PDSException is thrown for application level errors
     * @throws RemoteException RMI related errors
     */
    void fixDnsDomain( Organization organization, String oldDomainName ) throws PDSException, RemoteException;
    
    /**
     * copies a User
     *
     * @param sourceUserID
     * @return
     * @throws PDSException
     * Xdeprecated need to re-implement this with an eye to which settings
     * get copied.
     */
    User copyUser ( String sourceUserID ) throws PDSException, RemoteException;

    /**
     * returns a collection of Strings, each of which contains one of the
     * security roles for the given User.
     *
     * @param userID PK for the User whose roles are being queried
     * @return collection of Strings representing the User's security roles.
     * @throws PDSException is thrown for application level errors.
     */
    Collection getSecurityRoles ( String userID )
        throws PDSException, RemoteException;

    /**
     * assignSecurityRole gives the given User a new security role.
     *
     * @param userID userID PK for the User you are given the role to.
     * @param role security role name.  Currently 'SUPER', 'ADMIN' or
     * 'END_USER'.
     * @param password
     * @throws PDSException is thrown for application errors.
     */
    void assignSecurityRole( String userID,
                                    String role,
                                    String password ) throws PDSException, RemoteException;

    /**
     * assignDevice is used to make the given User the owner of the given Device.
     * Amongst other things this means that when projections are done for either
     * the User or Device the user-profile for the Device will pertain to this
     * owning User.
     *
     * @param deviceID PK of the Device that is to be owned by the User
     * @param userID PK of the User who is to own the given Device.
     * @throws PDSException is thrown for application errors.
     */
    void assignDevice( String deviceID, String userID)
        throws PDSException, RemoteException;

    /**
     * unassigns the given device from its current onwner.   It deletes the User and
     * Application profiles (for pingtel devices) from the ProfileWriter/CDS.
     *
     * @param deviceID PK of the Device you want to unassign
     * @throws PDSException is thrown for application errors
     */
    void unassignDevice(String deviceID ) throws PDSException, RemoteException;

    /**
     * restartDevices sends a signal to reset is sent to all of the Devices
     * owned by the given User.
     *
     * @param userID PK of the User whose Devices you want to reset.
     * @throws PDSException is thrown for application errors.
     */
    void restartDevices ( String userID ) throws PDSException, RemoteException;

    /**
     * generateProfiles create the profiles (Device configuration files) for
     * each of the Devices that the given User owns.
     *
     * @param userID PK of the User whose profiles you want to (re)create.
     * @param profileTypes a comma-separated list of profile types that
     * should be created for each of the User's Devices
     * @param projAlg fully qualified class name for the projection Class
     * to use.  If none is supplied then the default class is used for the
     * Device's DeviceType.
     * @throws PDSException is thrown for application errors
     */
    void generateProfiles(   String userID,
                                    String profileTypes,
                                    String projAlg) throws PDSException, RemoteException;

    /**
     * generateProfiles create the profiles (Device configuration files) for
     * each of the Devices that the given User owns.
     *
     * @param user User whose profiles you want to (re)create.
     * @param profileTypes a comma-separated list of profile types that
     * should be created for each of the User's Devices
     * @param projAlg fully qualified class name for the projection Class
     * to use.  If none is supplied then the default class is used for the
     * Device's DeviceType.
     * @throws PDSException is thrown for application errors
     */
    void generateProfiles(   User user,
                                    String profileTypes,
                                    String projAlg,
                                    boolean separateProjection) throws PDSException, RemoteException;

    /**
     * assignSecurityRole gives the given User a new security role.
     * (local)
     *
     * @param user User you are giving the role to.
     * @param role security role name.  Currently 'SUPER', 'ADMIN' or
     * 'END_USER'.
     * @param password
     * @throws PDSException is thrown for application errors.
     */
    void assignSecurityRole( User user,
                                    String role,
                                    String password ) throws PDSException, RemoteException;

    /**
     * NOTE: this is the 'local' implementation of the deleteUser,
     * the webUI should user the other version of this method.
     *
     * deleteUser implements the delete user use case.  It deletes
     * the user entity and all the user's dependent information
     * (configuration set, roles).  It doesn't delete any devices
     * that the user currently owns.
     *
     * @param user the EJBObject of the user to be deleted.
     * @throws PDSException is thrown for application level errors
     */
    void deleteUser( User user ) throws PDSException, RemoteException;

    /**
     * returns a digested value of the given password for the given User.
     *
     * @param displayID the user-visible ID for the User whose password is to be
     * calculated
     * @param dnsDomain the DNS Domain for the User (needed to create the
     * digest).
     * @param password plain-text password
     * @return digested password.
     */
    String digestUsersPassword ( String displayID, String dnsDomain, String password ) throws RemoteException;

    /**
     * generateUserProfiles generates the USER and APPLICATION profiles only
     * for a given Device.   The only place this code should be called from
     * is DeviceAdvocateBean.
     *
     * @param user EJBObject of User whose profiles you want to generate.
     * @param device EJBObject of the Device whose users profiles you want
     * to generate.
     * @param profileTypes comma-separated list of profile types.  Can only be
     * User and Application Ref
     * @param projectionAlg
     * @throws PDSException is thrown for non-critical errors.
     * @throws RemoteException for critical errors and remoting errors.
     * @see PDSDefinitions#PROF_TYPE_APPLICATION_REF
     * PDSDefinitions#PROF_TYPE_USER
     */
    void generateUserProfiles(  User user,
                                Device device,
                                String profileTypes,
                                String projectionAlg )
            throws PDSException, RemoteException;


}
