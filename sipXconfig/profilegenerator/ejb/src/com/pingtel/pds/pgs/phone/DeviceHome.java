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

import com.pingtel.pds.common.PDSException;

import java.rmi.RemoteException;
import java.util.Collection;

import javax.ejb.CreateException;
import javax.ejb.EJBHome;
import javax.ejb.FinderException;

/**
 * DeviceHome is the Home interface for the Device entity.
 *
 *@author ibutcher
 */
public interface DeviceHome extends EJBHome {

    /**
     * Standard  Entity Bean Method Implementation.
     *
     * @param shortName the display name by which the Device will be known
     * (shown in the user interfaces).
     * @param organizationID PK of the Organization that this Device belongs
     * to.
     * @param coreSoftwareDetailsID PK of the CoreSoftwareDetails that this
     * Device runs.  Not very interesting at the moment.
     * @param refConfigSetID PK of the RefConfigurationSet that this Device
     * is to have assigned to it.  This will ultimately define which
     * properties will end up this Device's profiles.
     * @param description short description of the Device, where is lives,
     * etc.
     * @param deviceGroupID PK of the DeviceGroup that this Device belongs
     * to.
     * @param userID PK of the User who owns this Device.
     * @param deviceTypeID PK of the DeviceType that is an instance of.
     * @param serialNumber the uniquely identifiying externally viewable
     * identity of this Device.  In the case of Pingtel and Cisco Devices
     * this is their MAC address.
     * @return PK of the new Device
     * @throws CreateException is thrown for non-fatal containter
     * generated errors (standard EJB pattern).
     * @throws PDSException is thrown for application level errors.
     */
    public Device create(   String shortName,
                            Integer organizationID,
                            Integer coreSoftwareDetailsID,
                            Integer refConfigSetID,
                            String description,
                            Integer deviceGroupID,
                            String userID,
                            Integer deviceTypeID,
                            String serialNumber )

             throws CreateException, RemoteException, PDSException;

    /**
     * Standard Entity Bean Finder method.
     *
     * @param pk Primary Key of the Device you wish to locate.
     * @return Device (EJBObject) of the matching Bean if it
     * exists.
     * @throws FinderException is thrown for non-fatal container generated
     * errors.
     * @throws RemoteException is thrown for system-level or remoting-
     * errors.
     */
    public Device findByPrimaryKey(Integer pk)
             throws FinderException, RemoteException;

    /**
     * Standard Entity Bean Finder method.
     * @return Collection of Device (EJBObject) for all Devices.
     * @throws FinderException is thrown for non-fatal container generated
     * errors.
     * @throws RemoteException is thrown for system-level or remoting-
     * errors.
     */
    public Collection findAll()
             throws FinderException, RemoteException;

    /**
     * locates Devices that belong to a given DeviceGroup.
     * @param deviceGroupID PK of the DeviceGroup whose Devices
     * you want to locate.
     * @return Collection of Device (EJBObject).
     * @throws FinderException is thrown for non-fatal container generated
     * errors.
     * @throws RemoteException is thrown for system-level or remoting-
     * errors.
     */
    public Collection findByDeviceGroupID(Integer deviceGroupID)
             throws FinderException, RemoteException;


    /**
     * locates Devices that belong to a given User
     * @param userID PK of the User whose Devices you want to find.
     * @return Collection of Device (EJBObject) that belong to the
     * given User.
     * @throws FinderException is thrown for non-fatal container generated
     * errors.
     * @throws RemoteException is thrown for system-level or remoting-
     * errors.
     */
    public Collection findByUserID(String userID)
             throws FinderException, RemoteException;


    /**
     * locates a Device which has the given short name.
     * At most one Device will be returned by this method.
     * @param shortName the name of the Device you want to locate.
     * @return Collection of Device (EJBObject), at most one,
     * for the given shortName.
     * @throws FinderException is thrown for non-fatal container generated
     * errors.
     * @throws RemoteException is thrown for system-level or remoting-
     * errors.
     */
    public Collection findByShortName(String shortName)
             throws FinderException, RemoteException;


//    public Collection findByRefConfigSetID(Integer refConfigSetID)
//             throws FinderException, RemoteException;

    /**
     * locates all of the Devices which belong to a particular
     * Organization.
     * @param organizationID the PK of the Organization whose
     * Devices you want to locate.
     * @return Collection of Device (EJBObject) for all of the
     * Devices in the Organization.
     * @throws FinderException is thrown for non-fatal container generated
     * errors.
     * @throws RemoteException is thrown for system-level or remoting-
     * errors.
     */
    public Collection findByOrganizationID(Integer organizationID)
             throws FinderException, RemoteException;

    /**
     * locates a Device which has the given serialNumber.
     * At most one Device will be returned by this method.
     * @param serialNumber the serialNumber of the Device whose
     * EJBean you want to locate.
     * @return Collection of Device (EJBObject), at most one,
     * for the given serialNumber.
     * @throws FinderException is thrown for non-fatal container generated
     * errors.
     * @throws RemoteException is thrown for system-level or remoting-
     * errors.
     */
    public Collection findBySerialNumber (String serialNumber )
             throws FinderException, RemoteException;


}

