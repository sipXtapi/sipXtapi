/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/profile/ProjectionHelperBusiness.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.profile;


import java.rmi.RemoteException;

import java.util.Collection;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.phone.Device;
import com.pingtel.pds.pgs.phone.DeviceGroup;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserGroup;


public interface ProjectionHelperBusiness {


    /**
     * projectAndPersist performs the 'projection' operation on the given set
     * of ProjectionInputs, transforms the output of this process using
     * XSLT and then sends the results to the ProfileWriter.
     *
     * @param projectionClassName fully-qualified class name to use for the
     * projection process.
     * @param device EJBObject for the Device whose profile(s) you want to
     * project.
     * @param profileType comma-separated list of profile types that you
     * want to project.
     * @param propertySets Collection of ProjectionInput Objects.
     * @throws PDSException is thrown for all application level errors.
     * @throws RemoteException is thrown for system or remoting errors.
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_USER
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_PHONE
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_APPLICATION_REF
     */
    void projectAndPersist( String projectionClassName,
                            Device device,
                            int profileType,
                            Collection propertySets)
             throws PDSException, RemoteException;

    /**
     * project performs the projection operation on a Collection of
     * ProjectionInput Objects.
     *
     * @param projectionClassName fully-qualified class name to use
     * for the projection process.
     * @param propertySets Collection of ProjectionInput Objects.
     * @param deviceTypeID PK of the DeviceType of the Device
     * where the resulting ProjectionInput object will be ultimately
     * delivered.
     * @param profileType type of profile the projection is being
     * projected.
     * @return a ProjectionInput which not contains the 'Projected'
     * values from the Collection of input ProjectionInputs.
     * @throws PDSException for application errors.
     * @throws RemoteException is thrown for system or remoting errors.
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_USER
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_PHONE
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_APPLICATION_REF
     */
    ProjectionInput project(String projectionClassName,
                            Collection propertySets,
                            Integer deviceTypeID,
                            int profileType )
            throws PDSException, RemoteException;

    /**
     * calculateTotalProfiles works out how many Devices belong to the given
     * DeviceGroup and all of its child DeviceGroups.
     * @param deviceGroup the DeviceGroup for whom you wish to calculate
     * the number of contained Devices.
     * @return the total number of Devices contained in the given group and all
     * of its child groups.
     * @throws RemoteException is thrown for system or remoting errors.
     */
    int calculateTotalProfiles (DeviceGroup deviceGroup) throws RemoteException;

    /**
     * returns the full-qualified class name which is the default
     * projection algorithm for the given Device's Device Type.
     * @param device EJBObject of the Device that you want the
     * projection class name for.
     * @param profileType projection algorithms are specifed at the
     * DeviceType profile type level.
     * @return fully-qualified class name.
     * @throws RemoteException is thrown for system or remoting errors.
     */
    String getProjectionClassName(Device device, int profileType)
             throws RemoteException;

    /**
     * calculateTotalProfiles works out how many Users belong to the given
     * UserGroup and all of its child UserGroups.
     * @param userGroup UserGroup whose total number of Users to be calculated
     * @return the total number of Users contained in the given group and all
     * of its child groups.
     */
    int calculateTotalProfiles (UserGroup userGroup) throws RemoteException;

    /**
     * addParentGroupConfigSets returns a Collection the ProjectionInput
     * associated with the given UserGroup and those of the groups parent
     * UserGroups.
     * @param userGroup EJBObject of the UserGroup whose ProjectionInputs
     * you want.
     * @param profileType Not Used really!!  Refactor.
     * @return Collection of ProjectionInputs.
     */
    Collection addParentGroupConfigSets( UserGroup userGroup, int profileType) throws RemoteException;

    /**
     * addParentGroupConfigSets returns a Collection the ProjectionInput
     * associated with the given DeviceGroup and those of the groups parent
     * DeviceGroups.
     * @param deviceGroup EJBObject of the DeviceGroup whose ProjectionInputs
     * you want.
     * @return Collection of ProjectionInputs.
     */
    Collection addParentGroupConfigSets(DeviceGroup deviceGroup) throws RemoteException;

    /**
     * getProjectionInput returns the ProjectionInput object for the
     * given User.
     * @param user EJBObject for the User whose ProjectionInput you want.
     * @param profileType Users effectively have two type of ProjectionInputs.
     * The User profile and the Application.   The User profile settings are
     * stored in the User's ConfigurationSet, the Application assignments are
     * calculated at run-time.
     * @return ProjectionInput for the User.
     */
    ProjectionInput getProjectionInput(User user, int profileType) throws RemoteException;

    /**
     * getProjectionInput returns the ProjectionInput object for the
     * given Device.
     * @param device EJBObject of the Device whose ProjectionInput
     * you want.
     * @return ProjectionInput for the Device.
     */
    ProjectionInput getProjectionInput(Device device) throws RemoteException;


}