/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/phone/DeviceGroupAdvocateBusiness.java#5 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.phone;


import java.rmi.RemoteException;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.PGSDefinitions;


public interface DeviceGroupAdvocateBusiness {

    /**
     * createDeviceGroup implements the create Device Group use
     * case.
     *
     *@param organizationID Organization PK (entity) that this
     * Device Group will belong to.
     *@param refConfigSetID Reference Configuration Set PK (entity)
     * which will be associated with the Device Group.
     *@param parentID PK of an existing Device Group (entity) which
     * you wish to become the parent (be the node immediately above) the
     * new Device Group.
     *@param childGroupIDs a comma-separeted list of Device Group PK
     * IDs. Each of these Device Groups will have the new Device Group become
     * their parent.
     *@param groupName name that the group will be known by.
     *@return The new Device Group's EJBObject.
     *@exception  PDSException  is thrown if there is an application level
     * errors.
     */
    DeviceGroup createDeviceGroup(   String organizationID,
                                            String groupName,
                                            String refConfigSetID,
                                            String parentID,
                                            String childGroupIDs)
        throws PDSException, RemoteException;

    /**
     * NOTE: This is the 'local' implementation.  The web UI should
     * use the other version of this method.
     *
     * createDeviceGroup implements the create Device Group use
     * case.
     *
     *@param organizationID Organization PK (entity) that this
     * Device Group will belong to.
     *@param refConfigSetID Reference Configuration Set PK (entity)
     * which will be associated with the Device Group.
     *@param parentID PK of an existing Device Group (entity) which
     * you wish to become the parent (be the node immediately above) the
     * new Device Group.
     *@param childGroupIDs a comma-separeted list of Device Group PK
     * IDs. Each of these Device Groups will have the new Device Group become
     * their parent.
     *@param groupName name that the group will be known by.
     *@return The new Device Group's EJBObject.
     *@exception  PDSException  is thrown if there is an application level
     * errors.
     */
    DeviceGroup createDeviceGroup(   Integer organizationID,
                                            String groupName,
                                            Integer refConfigSetID,
                                            Integer parentID,
                                            String childGroupIDs)
        throws PDSException, RemoteException;

    /**
     * deleteDeviceGroup implements the delete Device Group use case.
     *
     * @param deviceGroupID the PK of the DeviceGroup that you want
     * to delete.
     * @param option DeviceGroups can be delete in three ways:
     * PGSDefinitions.DEL_DEEP_DELETE is the only valid value currently.
     * @param newDeviceGroupID (unused, only use null)
     * @throws PDSException for application level errors.
     */
    void deleteDeviceGroup(  String deviceGroupID,
                                    String option,
                                    String newDeviceGroupID)
        throws PDSException, RemoteException;

    /**
     * NOTE: This is a 'local' method.   The web UI should use the
     * other version of this method.
     *
     * deleteDeviceGroup implements the delete Device Group use case.
     *
     * @param deviceGroup the EJBObject of the DeviceGroup that you want
     * to delete.
     * @param option DeviceGroups can be delete in three ways:
     * PGSDefinitions.DEL_DEEP_DELETE is the only valid value currently.
     * @param newDeviceGroupID (unused, only use null)
     * @throws PDSException for application level errors.
     */
    void deleteDeviceGroup(  DeviceGroup deviceGroup,
                                    int option,
                                    Integer newDeviceGroupID)
             throws PDSException, RemoteException;

    /**
     * editDeviceGroup implements the edit Device Group use case.
     * All of the parameters are optional except id.   If a null
     * value is provided for any of the optional values then one
     * of two things happens.  If the existing value is null then
     * nothing happens.  If the value is non-null then it is set
     * to null.
     *
     * @param id PK of the Device Group that you want to edit.
     * @param parentDeviceGroupID the PK of the Device Group that
     * you want to assign as the Device Groups parent (optional).
     * @param name the name by which the Device Group should be
     * known (optional).
     * @param refConfigSetID the PK of the RefConfigurationSet (entity)
     * that should be assigned to the DeviceGroup (optional).
     * @throws PDSException
     */
    void editDeviceGroup(    String id,
                                    String parentDeviceGroupID,
                                    String name,
                                    String refConfigSetID)
        throws PDSException, RemoteException;

    /**
     * Xdeprecated copyDeviceGroup implements the copy Device
     * Group use case.   The new Device Group is called:
     * 'copy of <source device group's name>'
     *
     * @param sourceDeviceGroupID the PK of the Device Group to be copied.
     * @return the EJBObject of the new copy of the Device Group
     * @throws PDSException is thrown for application level errors.
     */
    DeviceGroup copyDeviceGroup ( String sourceDeviceGroupID )
        throws PDSException, RemoteException;

    /**
     * restartDevices restarts all of the Devices which belong to the given
     * Device Group.
     *
     * @param deviceGroupID the PK of the Device Group whose Devices should
     * be restarted.
     * @throws PDSException is thrown for application level errors.
     * @see DeviceAdvocate
     */
    void restartDevices ( String deviceGroupID ) throws PDSException, RemoteException;

    /**
     *@param deviceGroupID the PK of the Device Group whose Devices should
     * have the profiles projected.
     *@param profileTypes comma-separated string of the profile types
     * to be generated for this Phone Tag. For the permissible values are
     * in PGSDefinitions - Macros for these values are ( PROF_TYPE_PHONE,
     * PROF_TYPE_USER, PROF_TYPE_APPLICATION_REF).
     *@param projAlg is the name of the class to be used for the
     * projection part of the operation (optional if a null is passed then
     * the default projection class for the phone type and profile is
     * used instead).
     *@exception PDSException  is thrown when an application level error
     * occurs.
     *@see PGSDefinitions
     */
    void generateProfiles(   String deviceGroupID,
                                    String profileTypes,
                                    String projAlg) throws PDSException, RemoteException;


}
