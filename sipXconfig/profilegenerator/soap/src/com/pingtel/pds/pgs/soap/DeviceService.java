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

package com.pingtel.pds.pgs.soap;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.phone.Device;
import com.pingtel.pds.pgs.phone.DeviceAdvocate;
import com.pingtel.pds.pgs.phone.DeviceAdvocateHome;
import com.pingtel.pds.pgs.phone.DeviceHome;
import com.pingtel.pds.pgs.phone.DeviceType;
import com.pingtel.pds.pgs.phone.DeviceTypeHome;

import javax.ejb.EJBException;
import javax.ejb.FinderException;
import java.rmi.RemoteException;
import java.util.Collection;
import java.util.Hashtable;

/**
 * DeviceService is an adapter class whose purpose is to
 * provide high, user friendly methods to do with Devices.   It is called
 * from the Axis SOAP servlet.
 */
public class DeviceService extends SoapService {

//////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final String VXWORKS_CORE_SOFTWARE_ID = "1";
    private static final String IX_WIN32_CORE_SOFTWARE_ID = "2";
    private static final String CISCO_7960_CORE_SOFTWARE_ID = "3";
    private static final String CISCO_7940_CORE_SOFTWARE_ID = "4";
    private static final String ORGANIZATION_ID_SIPX_ONLY = "1";
    private static final String COMPLETE_DEVICE_RCS_ID = "1";


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private DeviceAdvocate mDeviceAdvocateEJBObject;
    private DeviceTypeHome mDeviceTypeHome;
    private DeviceHome mDeviceHome;

    private static Hashtable mDeviceTypeIDModelMap = new Hashtable();


//////////////////////////////////////////////////////////////////////////
// Construction
////
    public DeviceService () {
        try {
            DeviceAdvocateHome deviceAdvocateHome =  (DeviceAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(DeviceAdvocateHome.class, "DeviceAdvocate");

            mDeviceAdvocateEJBObject = deviceAdvocateHome.create();

            mDeviceTypeHome = (DeviceTypeHome)
                    EJBHomeFactory.getInstance().getHomeInterface(DeviceTypeHome.class, "DeviceType");

            mDeviceHome = (DeviceHome) EJBHomeFactory.getInstance().getHomeInterface(DeviceHome.class, "Device");
        }
        catch (Exception e) {
            throw new EJBException(e.getMessage());
        }
    }


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * createDevice creates a new Device in SIPxchange.   The serialNumber
     * is the MAC address in the case of Pingtel and Cisco devices.
     * @param serialNumber is the MAC address in the case of Pingtel
     * and Cisco devices.
     * @param deviceModel is the name of the type of device.  The only
     * valid values are xpressa_strongarm_vxworks, ixpressa_x86_win32,
     * 7960 and 7940
     * @param deviceName the name that you wish to assign to this Device.
     * It is this value that you will see in the User Interface.
     * @param deviceGroupName the fully qualified name of the group
     * you want to put this new Device in.   You must put the full path
     * name in with '/' separating groups.
     * @param description an optional description for the Device.
     * @throws PDSException is thrown for all application level errors.
     */
    public void createDevice(   String serialNumber,
                                String deviceModel,
                                String deviceName,
                                String deviceGroupName,
                                String description)
            throws PDSException {

        checkMandatoryArgument("serialNumber", serialNumber);
        checkMandatoryArgument("deviceModel", deviceModel);
        checkMandatoryArgument("deviceName", deviceName);
        checkMandatoryArgument("deviceGroupName", deviceGroupName);

        String coreSoftwareDetailsID = null;

        if (deviceModel.equalsIgnoreCase(PDSDefinitions.MODEL_HARDPHONE_XPRESSA)) {
            coreSoftwareDetailsID = VXWORKS_CORE_SOFTWARE_ID;
        } else if (deviceModel.equalsIgnoreCase(PDSDefinitions.MODEL_SOFTPHONE_WIN)) {
            coreSoftwareDetailsID = IX_WIN32_CORE_SOFTWARE_ID;
        } else if (deviceModel.equalsIgnoreCase(PDSDefinitions.MODEL_HARDPHONE_CISCO_7960)) {
            coreSoftwareDetailsID = CISCO_7960_CORE_SOFTWARE_ID;
        } else if (deviceModel.equalsIgnoreCase(PDSDefinitions.MODEL_HARDPHONE_CISCO_7940)) {
            coreSoftwareDetailsID = CISCO_7940_CORE_SOFTWARE_ID;
        } else {
            throw new PDSException (mSoapStrings.getString("E_ILLEGAL_DEV_TYPE") +
                    PDSDefinitions.MODEL_HARDPHONE_XPRESSA + " " +
                    PDSDefinitions.MODEL_SOFTPHONE_WIN + " " +
                    PDSDefinitions.MODEL_HARDPHONE_CISCO_7960 + " " +
                    PDSDefinitions.MODEL_HARDPHONE_CISCO_7940);
        }

        try {
            mDeviceAdvocateEJBObject.createDevice(
                    deviceName,
                    ORGANIZATION_ID_SIPX_ONLY,
                    coreSoftwareDetailsID,
                    COMPLETE_DEVICE_RCS_ID,
                    description,
                    getDeviceGroupIDFromName(deviceGroupName),
                    null,
                    getDeviceTypeIDFromModel(deviceModel),
                    serialNumber);
        }
        catch (PDSException e) {
            throw e;
        }
        catch (Exception e) {
            throw new PDSException (e.getMessage());
        }
    }


    /**
     * deleteDevice removes a Device from SIPxchange.
     *
     * @param serialNumber the serial number of the Device to be
     * deleted.
     *
     * @throws PDSException is thrown for all application level errors.
     */
    public void deleteDevice (String serialNumber) throws PDSException {

        checkMandatoryArgument("serialNumber", serialNumber);

        try {
            Collection devices = mDeviceHome.findBySerialNumber(serialNumber);
            if(devices.isEmpty()){
                throw new PDSException(collateErrorMessages("UC432", "E1001", new Object[]{serialNumber}));
            }

            Device device = (Device) devices.iterator().next();

            mDeviceAdvocateEJBObject.deleteDevice(device);
        }
        catch (PDSException e) {
            throw e;
        }
        catch (Exception e) {
            throw new PDSException (e.getMessage());
        }
    }


    /**
     * editDevice allows you to modify an existing Device.   The Device you
     * wish to modify is specified by the existingSerialNumber parameter.  The
     * only mandatory parameter is the existingSerialNumber, you can supply null
     * for any of the others.
     *
     * @param existingSerialNumber the existingSerialNumber must correspond to
     * an existing Device in the Config Server.
     * @param newSerialNumber the purpose of newSerialNumber is to allow
     * you to change the serial number of an existing device.   Serial numbers
     * must be unique within a given Config Server.
     * @param name the purpose of the name field is to allow you to change
     * the name of an existing device.   Device names must be unique within a
     * given Config Server.
     * @param deviceGroupName if you supply a fully-qualified device group name
     * then the Device will be moved into this group.   You must specify the both the
     * ancestry and device group name relative to the root of the system.   For example
     * if I wanted to change the Devices' device group to the sales device group
     * which belongs to the (top) west coast user group I would supply
     * "west coast/sales".
     * @param description sets the description description of the Device.
     * You may supply a value of "NULL" or "null" to blank out the description
     * if it has been previoulsy set.
     *
     * @throws PDSException is thrown for all application level errors.
     */
    public void editDevice( String existingSerialNumber,
                            String newSerialNumber,
                            String name,
                            String deviceGroupName,
                            String description) throws PDSException {

        checkMandatoryArgument("existingSerialNumber", existingSerialNumber);

        try {
            Collection devices = mDeviceHome.findBySerialNumber(existingSerialNumber);
            if(devices.isEmpty()){
                throw new PDSException(collateErrorMessages("UC495", "E1001", new Object[]{existingSerialNumber}));
            }

            Device device = (Device) devices.iterator().next();

            String newSerialNumberParam = null;
            if (newSerialNumber == null){
                newSerialNumberParam = device.getSerialNumber();
            } else if (!newSerialNumber.equalsIgnoreCase(NULL_STRING)) {
                newSerialNumberParam = newSerialNumber;
            }

            String nameParam = null;
            if (name == null){
                nameParam = device.getShortName();
            } else if (!name.equalsIgnoreCase(NULL_STRING)) {
                nameParam = name;
            }

            String deviceGroupNameParam = null;
            if (deviceGroupName == null){
                deviceGroupNameParam = device.getDeviceGroupID().toString();
            } else if (deviceGroupName.equalsIgnoreCase(NULL_STRING)){
                throw new PDSException(collateErrorMessages("E3037", null));
            } else {
                deviceGroupNameParam = getDeviceGroupIDFromName(deviceGroupName);
            }

            String descriptionParam = null;
            if (description == null){
                descriptionParam = device.getDescription();
            } else if (!description.equalsIgnoreCase(NULL_STRING)){
                descriptionParam = description;
            }

            mDeviceAdvocateEJBObject.editDevice(    device.getID().toString(),
                                                    device.getUserID(),
                                                    deviceGroupNameParam,
                                                    device.getRefConfigSetID().toString(),
                                                    device.getCoreSoftwareDetailsID().toString(),
                                                    descriptionParam,
                                                    nameParam,
                                                    newSerialNumberParam);
        }
        catch (PDSException e) {
            throw e;
        }
        catch (Exception e) {
            throw new PDSException (e.getMessage());
        }
    }


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    private String getDeviceGroupIDFromName (String deviceGroupName) throws PDSException {
        return getDeviceGroupFromTree(deviceGroupName);
    }

    private String getDeviceTypeIDFromModel (String deviceModel)
            throws RemoteException, FinderException {

        if (!mDeviceTypeIDModelMap.contains(deviceModel)) {
            Collection groups = mDeviceTypeHome.findByModel(deviceModel);
            DeviceType deviceType = (DeviceType) groups.iterator().next();
            mDeviceTypeIDModelMap.put(deviceModel, deviceType.getID().toString());
        }

        return (String) mDeviceTypeIDModelMap.get(deviceModel);
    }

//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
