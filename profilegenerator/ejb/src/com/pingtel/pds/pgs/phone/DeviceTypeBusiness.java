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


public interface DeviceTypeBusiness {

     /**
     *  Gets the iD attribute of the DeviceType object
     *
     *@return                      The iD value
     *@exception  RemoteException  Description of the Exception
     */
    public Integer getID()
             throws RemoteException;


    /**
     *  Gets the manufacturerID attribute of the DeviceType object
     *
     *@return                      The manufacturerID value
     *@exception  RemoteException  Description of the Exception
     */
    public Integer getManufacturerID()
             throws RemoteException;


    /**
     *  Sets the manufacturerID attribute of the DeviceType object
     *
     *@param  manufacturerID       The new manufacturerID value
     *@exception  RemoteException  Description of the Exception
     */
    public void setManufacturerID(Integer manufacturerID)
             throws RemoteException;


    /**
     *  Gets the model attribute of the DeviceType object
     *
     *@return                      The model value
     *@exception  RemoteException  Description of the Exception
     */
    public String getModel()
             throws RemoteException;


    /**
     *  Sets the model attribute of the DeviceType object
     *
     *@param  model                The new model value
     *@exception  RemoteException  Description of the Exception
     */
    public void setModel(String model)
             throws RemoteException;

    public String getExternalID () throws RemoteException;

}