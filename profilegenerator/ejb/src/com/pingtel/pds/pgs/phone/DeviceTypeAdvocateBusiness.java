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

import javax.ejb.FinderException;
import javax.ejb.CreateException;

public interface DeviceTypeAdvocateBusiness {

    /**
     * importDeviceType is used during the post-installation process for
     * the config server.  It loads up device type definitions from the
     * various device type XML files and creates the associated entities
     * (ref properties, ref configuration sets, manufacturer details, ...).
     * It *does* use fileIO which is not part of the EJB spec however it
     * does so in a read-only way which means that it should be fine.
     * IMHO the spec is worried about non-TX file writes.
     *
     * @param fileName the name of the device definition XML file.
     * @return EJBObject for the new device type.
     * @throws PDSException is thrown for application level errors.
     */
    public DeviceType importDeviceType( String fileName )
            throws PDSException, RemoteException;

    /**
     * creates a device specific ref configuration set for the given device type and
     * organization.  These are used to produce a basic RCS for devices.
     * @param organization
     * @param deviceType
     */
    public void createDeviceSpecificRefConfigurationSet ( Organization organization,
                                                          DeviceType deviceType )
            throws RemoteException;

}