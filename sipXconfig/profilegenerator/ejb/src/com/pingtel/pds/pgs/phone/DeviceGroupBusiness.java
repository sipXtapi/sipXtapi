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


public interface DeviceGroupBusiness {

    public static final int MAX_NAME_LEN = 60;
    /**
     *  Gets the iD attribute of the DeviceGroup object
     *
     *@return                      The iD value
     *@exception  RemoteException  Description of the Exception
     */
    public Integer getID() throws RemoteException;


    /**
     *  Gets the parentID attribute of the DeviceGroup object
     *
     *@return                      The parentID value
     *@exception  RemoteException  Description of the Exception
     */
    public Integer getParentID() throws RemoteException;


    /**
     *  Sets the parentGroupID attribute of the DeviceGroup object
     *
     *@param  parentID   The new parentGroupID value
     *@exception  RemoteException  Description of the Exception
     */
    public void setParentID(Integer parentID)
             throws RemoteException;


    /**
     *  Gets the name attribute of the DeviceGroup object
     *
     *@return                      The name value
     *@exception  RemoteException  Description of the Exception
     */
    public String getName() throws RemoteException;


    /**
     *  Sets the name attribute of the DeviceGroup object
     *
     *@param  name                 The new name value
     *@exception  RemoteException  Description of the Exception
     */
    public void setName(String name) throws RemoteException, PDSException;


    /**
     *  Gets the organizationID attribute of the DeviceGroup object
     *
     *@return                      The organizationID value
     *@exception  RemoteException  Description of the Exception
     */
    public Integer getOrganizationID() throws RemoteException;


    /**
     *  Sets the refConfigSetID attribute of the DeviceGroup object
     *
     *@param  refConfigSetID       The new refConfigSetID value
     *@exception  RemoteException  Description of the Exception
     */
    public void setRefConfigSetID(Integer refConfigSetID)
             throws RemoteException;


    /**
     *  Gets the refConfigSetID attribute of the DeviceGroup object
     *
     *@return                      The refConfigSetID value
     *@exception  RemoteException  Description of the Exception
     */
    public Integer getRefConfigSetID() throws RemoteException;

    public String getExternalID () throws PDSException, RemoteException;

}
