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
 
package com.pingtel.pds.pgs.profile;

import java.rmi.RemoteException;
import java.util.Collection;

import com.pingtel.pds.common.PDSException;

public interface RefDataAdvocateBusiness {

    /**
     *  Description of the Method
     *
     *@param  name                  Description of the Parameter
     *@param  refPropertyGroupID    Description of the Parameter
     *@param  deviceTypeID           Description of the Parameter
     *@param  profileType           Description of the Parameter
     *@param  content               Description of the Parameter
     *@return                       Description of the Return Value
     *@exception  PDSException  Description of the Exception
     *@exception  RemoteException   Description of the Exception
     */
    public RefProperty createRefProperty(   String name,
                                            String profileType,
                                            String code,
                                            String content)
             throws PDSException, RemoteException;


    public RefProperty createRefProperty(   String name,
                                            int profileType,
                                            String code,
                                            String content)
        throws PDSException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  name                  Description of the Parameter
     *@param  organizationID        Description of the Parameter
     *@return                       Description of the Return Value
     *@exception  PDSException  Description of the Exception
     *@exception  RemoteException   Description of the Exception
     */
    public RefConfigurationSet createRefConfigurationSet(String name,
            String organizationID)
             throws PDSException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  refConfigSetID        Description of the Parameter
     *@exception  PDSException  Description of the Exception
     *@exception  RemoteException   Description of the Exception
     */
    public void deleteRefConfigSet(String refConfigSetID)
             throws PDSException, RemoteException;


    public void deleteRefConfigSet( RefConfigurationSet rcs )
             throws PDSException, RemoteException;

    /**
     *  Description of the Method
     *
     *@param  refConfigSetID        Description of the Parameter
     *@param  rule                  Description of the Parameter
     *@exception  PDSException  Description of the Exception
     *@exception  RemoteException   Description of the Exception
     */
    public void assignRefPropertytoRCS(String refConfigSetID,
            RefConfigSetAssignment rule)
             throws PDSException, RemoteException;

    public void assignRefPropertytoRCS( RefConfigurationSet rcs,
                                        RefConfigSetAssignment rule)
        throws PDSException, RemoteException;


    /**
     *@param  refConfigSetID        is the String representation of the PK of
     *      the Ref Config Set that you are trying to remove a Ref Property
     *      from.
     *@param  refPropertyID         is the String representation of the PK of
     *      the Ref Property that you are trying to remove from the Ref Config
     *      Set.
     *@exception  PDSException  is thrown if an application level error
     *      occurs.
     *@exception  RemoteException   is thrown if a system exception occurs.
     */
    public void unassignRefPropertyFromRCS(String refConfigSetID,
            String refPropertyID)
             throws PDSException, RemoteException;


    /**
     *  getRefConfigSetsProperties returns a Collection of P
     *
     *@param  refConfigSetID        the String representation of the PK of the
     *      Reference Configuration Set that you want to find which Reference
     *      Properties meta data for.
     *@return                       a Collection of PDSPropertyMetaData objects.
     *@exception  PDSException  is thrown for application level exceptions.
     *@exception  RemoteException   is thrown if a system exception occurs.
     *@see                          com.pingtel.pds.pgs.profile.PDSPropertyMetaData
     */
    public Collection getRefConfigSetsProperties(String refConfigSetID)
             throws PDSException, RemoteException;


    /**
     *  Gets the refConfigSetsProperty attribute of the RefDataAdvocate object
     *
     *@param  refConfigSetID        Description of the Parameter
     *@param  refPropertyID         Description of the Parameter
     *@return                       The refConfigSetsProperty value
     *@exception  PDSException  Description of the Exception
     *@exception  RemoteException   Description of the Exception
     */
    public ProjectionRule getRefConfigSetsProperty(String refConfigSetID,
            String refPropertyID)
             throws PDSException, RemoteException;


    public Collection getRefPropertiesForDeviceType ( Integer deviceTypeID )
        throws PDSException, RemoteException;

    public Collection getRefPropertiesForDeviceTypeAndProfile ( Integer deviceTypeID, Integer profileType )
        throws PDSException, RemoteException;

}
