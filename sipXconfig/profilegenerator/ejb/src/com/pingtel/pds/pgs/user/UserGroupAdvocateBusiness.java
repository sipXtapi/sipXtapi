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

import com.pingtel.pds.common.PDSException;


public interface UserGroupAdvocateBusiness {

    /**
     * createUserGroup implements the create user group use case.
     *
     * @param name the name that you want to give the new User Group.
     * @param parentID the PK of the user group that should be the
     * parent group of this user group (optional)
     * @param refConfigSetID the PK of the ref config set (Entity)
     * that will be the assigned to the user group.
     * @param organizationID the PK of the organization (Entity) that
     * will be assigned to the user group.
     * @return the EJBObject for the new user group.
     * @throws PDSException
     */
    UserGroup createUserGroup(   String name,
                                        String parentID,
                                        String refConfigSetID,
                                        String organizationID)
        throws PDSException, RemoteException;

    /**
     * NOTE: this is the local version of the use case, the web UI
     * should use the other version of this method.
     *
     * createUserGroup implements the create user group use case.
     *
     * @param name the name that you want to give the new User Group.
     * @param parentID the PK of the user group that should be the
     * parent group of this user group (optional)
     * @param refConfigSetID the PK of the ref config set (Entity)
     * that will be the assigned to the user group.
     * @param organizationID the PK of the organization (Entity) that
     * will be assigned to the user group.
     * @return the EJBObject for the new user group.
     * @throws PDSException
     */
    UserGroup createUserGroup(   String name,
                                        Integer parentID,
                                        Integer refConfigSetID,
                                        Integer organizationID)
        throws PDSException, RemoteException;



    /**
     * deleteUserGroup implements the delete user group use case.
     * Various other dependant entities are deleted as part of
     * the method.
     *
     * @param userGroupID the PK of the user group to delete.
     * @param option DeviceGroups can be delete in three ways:
     * PGSDefinitions.DEL_DEEP_DELETE is the only valid value currently.
     * @param newUserGroupID not currently used.
     * Xdeprecated newUserGroupID not currently used.
     * @throws PDSException is thrown for application level errors.
     */
    void deleteUserGroup(    String userGroupID,
                                    String option,
                                    String newUserGroupID)
        throws PDSException, RemoteException;

    /**
     * NOTE: this is the local implementation, the web UI should
     * use the other version of this method.
     *
     * deleteUserGroup implements the delete user group use case.
     * Various other dependant entities are deleted as part of
     * the method.
     *
     * @param ug the EJBObject of the user group to delete.
     * @param option DeviceGroups can be delete in three ways:
     * PGSDefinitions.DEL_DEEP_DELETE is the only valid value currently.
     * @param newUserGroupID not currently used.
     * Xdeprecated newUserGroupID not currently used.
     * @throws PDSException is thrown for application level errors.
     */
    void deleteUserGroup(    UserGroup ug,
                                    int option,
                                    Integer newUserGroupID)
        throws PDSException, RemoteException;

    /**
     * copyUserGroup implements the copy user group use case.
     * Xdeprecated this method should not be used for now.
     * @param sourceUserGroupID the PK of the user group that
     * should be copied.
     * @return the EJBObject of the new (copy) user group.
     * @throws PDSException is thrown for application level errors.
     */
    UserGroup copyUserGroup ( String sourceUserGroupID )
        throws PDSException, RemoteException;

    /**
     * restarts all devices of all of the users who are in the given
     * user group.   This is also recursive, if the user group is the
     * parent of other user groups then they will be restarted.
     *
     * @param userGroupID the PK of the user group to restart.
     * @throws PDSException is thrown for application level errors.
     */
    void restartDevices ( String userGroupID ) throws PDSException, RemoteException;

    /**
     * generateProfiles implements the project profile use case
     * for a user group.   This is also recursive, if the user group is the
     * parent of other user groups then they will be projected.
     *
     * @param userGroupID the PK of the user group whose profiles
     * should be generated.
     * @param profileTypes comma-separated string of the profile types
     * to be generated for this User Group.  For the permissible values are
     * in PGSDefinitions - definitions for these values are ( PROF_TYPE_PHONE,
     * PROF_TYPE_USER, PROF_TYPE_APPLICATION_REF).
     * @param projAlg is the name of the class to be used for the
     * projection part of the operation (optional if a null is passed then
     * the default projection class for the phone type and profile is
     * used instead).
     * @throws PDSException is thrown for application level errors.
     */
    void generateProfiles(   String userGroupID,
                                    String profileTypes,
                                    String projAlg )
        throws PDSException, RemoteException;

    /**
     * editUserGroup implements the edit user group use case.
     * It provides a session facade to set the attributes of
     * a user group.    All parameters are optional with the exception
     * of userGroupID.   If a null is passed for a parameter then
     * one of two things happens.   If the existing attribute value
     * is null then nothing happens.   If the existing attribute value
     * is not null then it is set to null.
     *
     * @param userGroupID the PK of user group that you wish to edit.
     * @param name the new name for the user group (optional).
     * @param parentID the PK of the new parent user group (optional).
     * @param refConfigSetID the PK of the new ref config set (optional).
     * @throws PDSException is thrown for application level errors.
     */
    public boolean [] editUserGroup(    String userGroupID,
                                        String name,
                                        String parentID,
                                        String refConfigSetID)
            throws PDSException, RemoteException;


}
