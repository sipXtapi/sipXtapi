/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/phone/DeviceBusiness.java#4 $
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


public interface DeviceBusiness {

    public static final int MAX_SHORT_NAME_LEN = 60;
    public static final int MAX_DESCRIPTION_LEN = 100;
    public static final int MAX_SERIAL_NUMBER_LEN = 12;

    /**
     * @return  the value of Devices' id (it's PK)
     * @throws RemoteException for system- and remoting- errors.
     */
    public Integer getID() throws RemoteException;

    /**
     * getOrganization returns the Organization EJBObject of the Organization
     * that this Device belongs to.
     *
     * @return Organization EJBObject which owns this Device.
     * @throws RemoteException for system- and remoting- errors.
     */
    public Integer getOrganizationID()
            throws RemoteException;

    /**
     * gets the shortName attribute of the Device
     *
     * @return shortName of the Device
     * @throws RemoteException for system- and remoting- errors.
     */
    public String getShortName()
            throws RemoteException;

    /**
     * sets the shortName attribute of the LogicalPhoneBean object
     *
     * @param shortName the new shortName value
     * @throws RemoteException for system- and remoting- errors.
     */
    public void setShortName(String shortName)
            throws PDSException, RemoteException;

    /**
     * returns the userID of this Device.
     *
     * @return the value of the User who own's this Device
     * @throws RemoteException for system- and remoting- errors.
     */
    public String getUserID()
            throws RemoteException;

    /**
     * sets the userID of this Device.
     *
     * @param userID the primary key of the User who is to own this Device
     * @throws RemoteException for system- and remoting- errors.
     */
    public void setUserID(String userID)
            throws RemoteException;

    /**
     * gets the DeviceGroupID of this Device.
     *
     * @return the DeviceGroupID which this Device belongs to
     * @throws RemoteException for system- and remoting- errors.
     */
    public Integer getDeviceGroupID()
            throws RemoteException;

    /**
     * sets the DeviceGroupID for this Device.
     *
     * @param  deviceGroupID  the PK of the DeviceGroup which you wish to
     * add this Device to.
     * @throws RemoteException for system- and remoting- errors.
     */
    public void setDeviceGroupID(Integer deviceGroupID)
             throws RemoteException, PDSException;

    /**
     * gets the refConfigSetID attribute of the Device.
     *
     * @return The current refConfigSetID
     * @throws RemoteException for system- and remoting- errors.
     */
    public Integer getRefConfigSetID()
            throws RemoteException;


    /**
     * sets the refConfigurationSetID for this Device.
     *
     * @param refConfigSetID  the PK of the RefConfigurationSet that
     * you wish to assocatied with this Device.
     * @throws RemoteException for system- and remoting- errors.
     */
    public void setRefConfigSetID(Integer refConfigSetID)
             throws PDSException, RemoteException;

    /**
     * sets the coreSoftwareDetailsID for this Device.
     *
     * @param  coreSoftwareDetailsID  the primary key of coreSoftwareDetails
     * (version of OS or firmware to run on the LogicalPhone) that you wish
     * to associate with the Device
     * @throws RemoteException for system- and remoting- errors.
     */
    public void setCoreSoftwareDetailsID(Integer coreSoftwareDetailsID)
             throws PDSException, RemoteException;

    /**
     * gets the coreSoftwareDetailsID for this Device.
     *
     * @return the PK of the CoreSoftwareDetails currently
     * asssociated with the LogicalPhone.
     * @throws RemoteException for system- and remoting- errors.
     */
    public Integer getCoreSoftwareDetailsID()
             throws RemoteException;

    /**
     * sets the description for this Device.
     *
     * @param description some free-text that you want to use to label this
     * Device.
     * @throws RemoteException for system- and remoting- errors.
     */
    public void setDescription(String description)
             throws PDSException, RemoteException;

    /**
     * gets the description for this Device.
     *
     *@return the description of this Device.
     * @throws RemoteException for system- and remoting- errors.
     */
    public String getDescription()
             throws RemoteException;

    /**
     * returns a 'friendly' identity of the Device.  This is userful for identifying
     * the Device to users of the system.
     *
     * @return friendly identity/description of the Device.
     * @throws RemoteException for system- and remoting- errors.
     */
    public String getExternalID ()
            throws RemoteException;

    /**
     * gets the deviceTypeID for this Device.
     *
     * @return the deviceTypeID of this Device.
     * @throws RemoteException for system- and remoting- errors.
     */
    public Integer getDeviceTypeID()
            throws RemoteException;

    /**
     * gets the serialNumber for this Device.
     *
     * @return the serialNumber of this Device.
     * @throws RemoteException for system- and remoting- errors.
     */
    public String getSerialNumber()
            throws RemoteException;

    /**
     * setSerialNumber sets the serialNumber for this Device.
     *
     * @param serialNumber the new serialNumber you wish to assign to this Device.
     *
     * @throws PDSException is thrown for a variety of validation errors such as using
     * a serialNumber associated with another phone, the serialNumber being too long
     * or containing invalid characters.
     * @throws RemoteException for system- and remoting- errors.
     */
    public void setSerialNumber (String serialNumber)
            throws PDSException, RemoteException;

    /**
     * calculateDeviceLineURL works out the 'DEVICE_LINE' SIP URL
     * for Pingtel Devices.
     *
     * @return a String value of the Device Line SIP URL.
     * @throws PDSException is thrown for application level errors.
     * @throws RemoteException for system- and remoting- errors.
     */
    public String calculateDeviceLineURL ()
            throws PDSException, RemoteException;

    /**
     * returns the model for this devices device type.
     * @return returns the model for this devices device type
     * @throws RemoteException for system- and remoting- errors.
     */
    public String getModel () throws RemoteException;

    /**
     * returns the manufacturer name of this devices
     * @return manufacturer name of this devices
     * @throws RemoteException for system- and remoting- errors.
     */
    public String getManufaturerName () throws RemoteException;

    /**
     * getOrganization returns the Organization EJBObject of the Organization
     * that this Device belongs to.
     *
     * @return Organization EJBObject which owns this Device.
     * @throws PDSException is thrown for application level errors.
     * @throws RemoteException for system- and remoting- errors.
     */
    Organization getOrganization() throws PDSException, RemoteException;

}