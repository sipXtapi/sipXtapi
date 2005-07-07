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
import java.util.Collection;

import javax.ejb.CreateException;
import javax.ejb.EJBHome;
import javax.ejb.FinderException;

/**
 *  Description of the Interface
 *
 *@author     ibutcher
 *@created    December 13, 2001
 */
public interface DeviceTypeHome extends EJBHome {

    /**
     *  Description of the Method
     *
     *@param  manufacturerID       Description of the Parameter
     *@param  model                Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  CreateException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public DeviceType create(Integer manufacturerID, String model)
             throws CreateException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  pk                   Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public DeviceType findByPrimaryKey(Integer pk)
             throws FinderException, RemoteException;


    /**
     *  Description of the Method
     *
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public Collection findAll()
             throws FinderException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  model                Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public Collection findByModel(String model)
             throws FinderException, RemoteException;

}
