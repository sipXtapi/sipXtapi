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
package com.pingtel.pds.pgs.phone;

import java.rmi.RemoteException;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.organization.Organization;

public interface DeviceAdvocateBusiness {


    /**
     * createDevice models the create device use case.
     *
     * @param shortName is a short textual identifier.
     * @param organizationID Organization to which this Device belongs.
     * @param coreSoftwareDetailsID the PK of the core software (Entity)
     * which this device uses.
     * @param refConfigSetID PK of the Reference Configuration Set (Entity)
     * which should be associated with this Device (optional).
     * @param description a long free text field (optional).
     * @param deviceGroupID the PK of a DeviceGroup (entity) that this
     * Device should be assigned to (optional).
     * @param userID the PK of a User (entity) that is the owner of
     * this Device.
     * @param deviceTypeID the PK of the DeviceType (entity) that this
     * Device is an instance of (Pingtel VXWorks, NT, etc.)
     * @param serialNumber the unique manufacturers serial number for
     * this device.  For Pingtel phones this is their MAC address.
     *
     * @return the EJBObject for the new Device
     * @exception PDSException is thrown if an Application type error
     * occurs
     */
    public Device createDevice( String shortName,
                                String organizationID,
                                String coreSoftwareDetailsID,
                                String refConfigSetID,
                                String description,   // nullable
                                String deviceGroupID,  // nullable
                                String userID,
                                String deviceTypeID,
                                String serialNumber ) throws PDSException, RemoteException;

    /**
     * NOTE: This is the 'local' version of the create device use case.
     * The web UI should use the other version of this method.
     *
     * createDevice models the create device use case.
     *
     * @param shortName is a short textual identifier.
     * @param organizationID Organization to which this Device belongs.
     * @param coreSoftwareDetailsID the PK of the core software (Entity)
     * which this device uses.
     * @param refConfigSetID PK of the Reference Configuration Set (Entity)
     * which should be associated with this Device (optional).
     * @param description a long free text field (optional).
     * @param deviceGroupID the PK of a DeviceGroup (entity) that this
     * Device should be assigned to (optional).
     * @param userID the PK of a User (entity) that is the owner of
     * this Device.
     * @param deviceTypeID the PK of the DeviceType (entity) that this
     * Device is an instance of (Pingtel VXWorks, NT, etc.)
     * @param serialNumber the unique manufacturers serial number for
     * this device.  For Pingtel phones this is their MAC address.
     *
     * @return the EJBObject for the new Device
     * @exception PDSException is thrown if an Application type error
     * occurs
     */
    public Device createDevice(    String shortName,
                            Integer organizationID,
                            Integer coreSoftwareDetailsID,
                            Integer refConfigSetID,
                            String description,
                            Integer deviceGroupID,
                            String userID,
                            Integer deviceTypeID,
                            String serialNumber ) throws PDSException, RemoteException;

    /**
     * Deletes the Device object and its associated entities.
     *
     * @param deviceID the PK of the device to be deleted
     *
     * @exception PDSException is thrown for application level
     * errors.
     */
    public void deleteDevice( String deviceID ) throws PDSException, RemoteException;

    /**
     * NOTE: This is the 'local' version of the create device use case.
     * The web UI should use the other version of this method.
     *
     * Deletes the Device object and its associated entities.
     *
     * @param device the EJBObject of the device to be deleted
     *
     * @exception PDSException is thrown for application level
     * errors.
     */
    public void deleteDevice( Device device ) throws PDSException, RemoteException;

    /**
     * Xdeprecated copyDevice implements the copy device use case.
     * This is currently deprecated.  The copied device is called
     * 'copy of <source devices name>'.
     * @param sourceDeviceID the PK of the device to be copied.
     * @return the EJBObject of the new (copied) device.
     * @throws PDSException
     */
    public Device copyDevice(String sourceDeviceID) throws PDSException, RemoteException;

    /**
     * editDevice implements the edit device use case.   All parameters
     * are optional except the id.  If a null value is passed for any
     * of the optional parameters then one of two things happens.  If
     * the existing value for the Device is null then nothing happens.
     * If there is an existing value then it will be set to null.
     *
     * @param id the PK of the Device that is to be edited.
     * @param userID the PK of the User (entity) that you wish to assign
     * to this Device.
     * @param deviceGroupID the PK of the DeviceGroup (entity) that you
     * wish to assign to this Device.
     * @param refConfigSetID the PK of the RefConfigSet (entity) that you
     * wish to assign to this Device.
     * @param coreSoftwareDetailsID the PK of the CoreSoftwareDetails
     * (entity) that you wish to assign to this Device.
     * @param description free text description for the Device.
     * @param shortName a unique display name associated with the device.
     * @param serialNumber the unique manufacturers serial number for
     * this device.  For Pingtel phones this is their MAC address.
     * @throws PDSException for application level errors.
     */
    public boolean [] editDevice(   String id,
                                    String userID,
                                    String deviceGroupID,
                                    String refConfigSetID,
                                    String coreSoftwareDetailsID,
                                    String description,
                                    String shortName,
                                    String serialNumber ) throws PDSException, RemoteException;

    /**
     * restartDevice sends a signal to an managed Device to tell it to
     * reset itself.  The PGS delegates the work to the SDS (via its
     * RMI interface) as the SDS can talk SIP.
     *
     * @param deviceID the PK of the Device to be reset
     * @throws PDSException for application level errors.
     */
    public void restartDevice ( String deviceID )
        throws PDSException, RemoteException;

    /**
     * generateProfiles creates the Device's profiles (configuration files)
     * which are written to disk by the ProfileWriter (which is called over
     * it's RMI interface).   If the Device is currently enrolled then it
     * is notified that new profiles exist for it.
     *
     * @param deviceID the PK of the Device which is to be projected.
     * @param profileTypes a comma-separated list of profile types.  The
     * valid values are PDSDefinitions.PROF_TYPE_USER,
     * PDSDefinitions.PROF_TYPE_DEVICE and
     * PDSDefinitions.PROF_TYPE_APPLICATION_REF.
     * @param projectionAlgorithm the fully qualified class name for the
     * projection algorithm (optional).  If none is supplied then the
     * default algorithm is used.
     * @throws PDSException is thrown for application level errors.
     */
    public void generateProfiles(   String deviceID,
                                    String profileTypes,
                                    String projectionAlgorithm) throws PDSException, RemoteException;

    /**
     * NOTE: This is the 'local' version of the generate profiles use case.
     * The web UI should use the other version of this method.
     *
     * generateProfiles creates the Device's profiles (configuration files)
     * which are written to disk by the ProfileWriter (which is called over
     * it's RMI interface).   If the Device is currently enrolled then it
     * is notified that new profiles exist for it.
     *
     * @param device the EJBObject of the Device which is to be projected.
     * @param profileTypes a comma-separated list of profile types.  The
     * valid values are PDSDefinitions.PROF_TYPE_USER,
     * PDSDefinitions.PROF_TYPE_DEVICE and
     * PDSDefinitions.PROF_TYPE_APPLICATION_REF.
     * @param projectionAlgorithm the fully qualified class name for the
     * projection algorithm (optional).  If none is supplied then the
     * default algorithm is used.
     * @throws PDSException is thrown for application level errors.
     */
    public void generateProfiles(   Device device,
                                    String profileTypes,
                                    String projectionAlgorithm,
                                    boolean separateProjection)
            throws PDSException, RemoteException;

    /**
     * fetchSequenceNumber is used by the SDS when a device enrolls with it
     * to discover the newest version of each profile for that device.
     *
     * @param serialNumber the serial number (MAC address for Pingtel phones)
     * of the Device you want the sequence number for.
     * @param profileType valid values are PDSDefinitions.PROF_TYPE_USER,
     * PDSDefinitions.PROF_TYPE_DEVICE and
     * PDSDefinitions.PROF_TYPE_APPLICATION_REF.
     * @return the sequence number for the given Device's profile type.
     * @throws PDSException for application level errors.
     */
    public int fetchSequenceNumber( String serialNumber, int profileType)
         throws PDSException, RemoteException;
    
    
    /**
     * Our data model stores some data - such as organization DNS name - in multiple places.
     * Every time organization name is changes one needs to go through all devices and 
     * fix internal device data to refer to a new name.
     * @param organization reference to a bean which DNS domain name has been changed
     * @throws PDSException
     * @throws RemoteException
     */
    public void fixDnsName( Organization organization ) 
        throws PDSException, RemoteException;

}
