/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/user/ApplicationAdvocateBusiness.java#4 $
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

public interface ApplicationAdvocateBusiness {
    /**
     * assignApplicationToUser is used to add an Application to the list of
     * Applications that get sent to a User's Devices in the Application
     * profile.
     *
     * @param userID PK of the User whom you wish to add the Application
     * to.
     * @param applicationID PK of the Application that you wish to assign
     * to the User.
     * @throws PDSException is thrown for application errors.
     */
    void assignApplicationToUser (String userID, String applicationID )
        throws PDSException, RemoteException;

    /**
     * assignApplicationToUserGroup is used to add an Application to a UserGroup.
     * The Application is added to the list of Applications that get sent to a
     * each of the User's Devices in the given UserGroup.
     *
     * @param userGroupID PK of the UserGroup that you want to add the Application
     * to.
     * @param applicationID PK of the Application that is to be added to the
     * UserGroup
     * @throws PDSException is thrown for application errors.
     */
    void assignApplicationToUserGroup ( String userGroupID, String applicationID )
        throws PDSException, RemoteException;

    /**
     * unassignAllUsersApplications removes all of the Applications from the given
     * User's application profile.
     *
     * @param userID PK of the User whom you want to remove the Applications from.
     * @throws PDSException is thrown for all application errors.
     */
    void unassignAllUsersApplications ( String userID )
        throws PDSException, RemoteException;

    /**
     * unassignAllUserGroupsApplications removes all of the Applications from
     * the application profile's of all of the Users in the given UserGroup.
     *
     * @param userGroupID PK of the UserGroup whom you wish to remove the
     * Applications from.
     * @throws PDSException is thrown for application errors.
     */
    void unassignAllUserGroupsApplications ( String userGroupID )
        throws PDSException, RemoteException;

    /**
     * installAndAssignForUser creates a new Application and assigns it to the
     * given User.
     *
     * @param name the name by which the new Application will be known to
     * users of the sytem.
     * @param organizationID PK of the Organization which this Application
     * will belong to.
     * @param deviceTypeID PK of the DeviceType that this Application is
     * available to.
     * @param refPropertyID PK of the RefProperty which this Application is
     * and instance of.  Currently all Applications should be created using
     * the Pingtel 'USER' RefProperty.   When we get around to it there are
     * other types off Applications that can run on the the phone such as
     * 'VoiceMail'.   We differentiate Applications using different
     * RefProperties because their cardinalities may be different.
     * Reason, we don't want to have Applications competing for resources,
     * such as VoiceMail or CallLogging.
     * @param url URL that will be sent to the Devices as being the location
     * of the the Application.
     * @param description description of the new Application.
     * @throws PDSException
     * @see #assignApplicationToUser
     */
    void installAndAssignForUser(   String userID,
                                    String name,
                                    String organizationID,
                                    String deviceTypeID,
                                    String refPropertyID,
                                    String url,
                                    String description )
        throws PDSException, RemoteException;

    /**
     * uninstallApplicationFromUser removes the given Application from the
     * User's application profile.
     *
     * @param userID PK of the User you want to remove the Application from
     * @param applicationID PK of the Application that you want to remove
     * from the User's application profile.
     * @throws PDSException is thrown for application errors.
     */
    void uninstallApplicationFromUser ( String userID, String applicationID )
            throws PDSException, RemoteException;

    /**
     * uninstallApplicationFromUserGroup removes the given Application from
     * the application profiles of all of the Users in the given UserGroup.
     *
     * @param userGroupID PK of the UserGroup who you want to remove the
     * Application from.
     * @param applicationID PK of the Application that you are removing from
     * the UserGroup.
     * @throws PDSException is thrown for application errors.
     */
    void uninstallApplicationFromUserGroup ( String userGroupID, String applicationID )
        throws PDSException, RemoteException;

    /**
     * deleteApplication removes an Application and all it's assignments to
     * User and User Groups, from the system.
     *
     * @param applicationID
     * @throws PDSException
     */
    void deleteApplication ( String applicationID )
        throws PDSException, RemoteException;

    /**
     * editApplication modifies the attributes of an existing Application.
     *
     * @param applicationID PK of the Application you want to modify.
     * @param name the new name that you want to call the Application.
     * @param url the new URL that you want to use for the Application.
     * @param description the new description you want to use for the
     * Application.
     * @throws PDSException is thrown for application errors.
     */
    void editApplication (  String applicationID,
                            String name,
                            String url,
                            String description )
        throws PDSException, RemoteException;

    /**
     * createApplication create a new Application in the sytem.
     *
     * @param name the name by which the new Application will be known to
     * users of the sytem.
     * @param organizationID PK of the Organization which this Application
     * will belong to.
     * @param deviceTypeID PK of the DeviceType that this Application is
     * available to.
     * @param refPropertyID PK of the RefProperty which this Application is
     * and instance of.  Currently all Applications should be created using
     * the Pingtel 'USER' RefProperty.   When we get around to it there are
     * other types off Applications that can run on the the phone such as
     * 'VoiceMail'.   We differentiate Applications using different
     * RefProperties because their cardinalities may be different.
     * Reason, we don't want to have Applications competing for resources,
     * such as VoiceMail or CallLogging.
     * @param url URL that will be sent to the Devices as being the location
     * of the the Application.
     * @param description description of the new Application.
     * @throws PDSException
     */
    Application createApplication ( String name,
                                    String organizationID,
                                    String deviceTypeID,
                                    String refPropertyID,
                                    String url,
                                    String description )
        throws PDSException, RemoteException;


}