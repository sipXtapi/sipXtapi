/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/profile/RenderProfileBusiness.java#4 $
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

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.phone.Device;

public interface RenderProfileBusiness {

    /**
     * deleteProfile sends a request to the ProfileWriter to remove a profile
     * of the given profile type for the given Device from it's persistent
     * store.
     *
     * @param device Device whose profile should be deleted.
     * @param profileType type of profile this bytestream represents, see
     * PDSDefintions PROF_ values.
     * @exception PDSException is thrown for application errors.
     * @exception RemoteException is thrown for system or remoting errors.
     * @see com.pingtel.pds.common.PDSDefinitions
     */
    public void deleteProfile( Device device, int profileType )
        throws PDSException, RemoteException;

    /**
     *  Sends a PreRenderedProfile object to the RMI service where it is
     *  persisted to disk and made available for download via the SDS
     *
     * @param transformedProfile actual bytestream content of the profile
     * to be written.
     * @param device Device that this profile belongs to
     * @param profileType type of profile this bytestream represents, see
     * PDSDefintions PROF_ values.
     * @exception PDSException is thrown for application errors.
     * @exception RemoteException is thrown for system or remoting errors.
     * @see com.pingtel.pds.common.PDSDefinitions
     */
    void writeProfile( byte[] transformedProfile, Device device, int profileType )
        throws PDSException, RemoteException;

    /**
     * getDeviceNotifyURL returns the URL which is used in the NOTIFY/check-sync
     * message which is sent to the Cisco 79XX devices.
     *
     * @param device Device for whom you want the URL
     * @return URL to be used in the NOTIFY
     * @throws PDSException is thrown for non-system errors.
     * @throws RemoteException is thrown for system/remoting errors.
     */
    String getDeviceNotifyURL ( Device device )
            throws PDSException, RemoteException;

}