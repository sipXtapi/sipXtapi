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

package com.pingtel.pds.pgs.organization;

import java.rmi.RemoteException;


public interface OrganizationBusiness {

    /**
     *  Gets the iD attribute of the Organization object
     *
     *@return                      The iD value
     *@exception  RemoteException  Description of the Exception
     */
    public Integer getID() throws RemoteException;


    /**
     *  Gets the name attribute of the Organization object
     *
     *@return                      The name value
     *@exception  RemoteException  Description of the Exception
     */
    public String getName() throws RemoteException;


    /**
     *  Sets the name attribute of the Organization object
     *
     *@param  name                 The new name value
     *@exception  RemoteException  Description of the Exception
     */
    public void setName(String name) throws RemoteException;


    /**
     *  Sets the parentID attribute of the Organization object
     *
     *@param  parentID             The new parentOrganizationID value
     *@exception  RemoteException  Description of the Exception
     */
    public void setParentID(Integer parentID)
             throws RemoteException;


    /**
     *  Gets the parentID attribute of the Organization object
     *
     *@return                      The parentOrganizationID value
     *@exception  RemoteException  Description of the Exception
     */
    public Integer getParentID() throws RemoteException;

    public String getDNSDomain () throws RemoteException;

    public void setDNSDomain ( String dnsDomain ) throws RemoteException;

    public int getStereotype () throws RemoteException;

    public String getExternalID () throws RemoteException;
}