/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/phone/DeviceGroupHome.java#4 $
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
 * DeviceGroupHome is the home interface for the DeviceGroup entity bean.
 *
 * @author IB
 * @see com.pingtel.pds.pgs.phone.DeviceGroup
 * @see com.pingtel.pds.pgs.phone.DeviceGroupBean
 */
public interface DeviceGroupHome extends EJBHome {

    /**
     * standard EJ Bean create method
     *
     *@param name name that will be given to the new DeviceGroup
     *@param organizationID PK of the Organization that this DeviceGroup
     * is being created into
     *@param  refConfigSetID PK of the RefConfigurationSet that this
     * DeviceGroup will use
     *@return DeviceGroup EJBObject for the new DeviceGroup
     *@exception CreateException for application level errors
     *@exception RemoteException system errors
     */
    public DeviceGroup create(  String name,
                                Integer organizationID,
                                Integer refConfigSetID)
            throws CreateException, RemoteException, PDSException;


    /**
     * Standard EJB implementation
     *
     * @param pk is the primary key of the DeviceGroup that you are trying
     * to locate.
     * @return DeviceGroup is the EJBObject proxy/remote interface to the
     * located bean.
     * @exception RemoteException is thrown if a System exception occurs.
     * @exception FinderException is thrown for application level errors.
     */
    public DeviceGroup findByPrimaryKey(Integer pk)
             throws FinderException, RemoteException;


    /**
     * finds all the DeviceGroups whose parent DeviceGroup matches the
     * input parameter.
     *
     * @param  parentID is the PK of the DeviceGroup whose children you
     * are trying to locate
     * @return a collection of DeviceGroup EJBObjects of the beans located.
     * @exception RemoteException is thrown if a System exception occurs.
     * @exception FinderException is thrown for application level errors.
     */
    public Collection findByParentID(Integer parentID)
             throws FinderException, RemoteException;


    /**
     * finds all the DeviceGroups whose name matches the input parameter.
     *
     * @param name is the name of the DeviceGroup that you are trying to locate.
     * Note that this will return > 1 DeviceGroup if that
     * name appears in > 1 Organizations.
     * @return a collection of DeviceGroup EJBObjects of the beans located.
     * @exception RemoteException is thrown if a System exception occurs.
     * @exception FinderException is thrown for application level errors.
     */
    public Collection findByName(String name)
             throws FinderException, RemoteException;


    /**
     * finds all the DeviceGroups in an Organization.
     *
     * @param  organizationID is the PK of the Organization/Domain of the
     * DeviceGroup that you are trying to locate.
     * @return a collection of DeviceGroup EJBObjects of the beans located.
     * @exception RemoteException is thrown if a System exception occurs.
     * @exception FinderException is thrown for application level errors.
     */
    public Collection findByOrganizationID(Integer organizationID)
             throws FinderException, RemoteException;


    /**
     * finds all the DeviceGroups whose refConfigSetID matches the input
     * parameter.
     *
     * @param refConfigSetID the PK of the RefConfigurationSet which is used
     * to find DeviceGroups whose refConfigSetID matches
     * @return a collection of DeviceGroup EJBObjects of the beans located.
     * @exception RemoteException is thrown if a System exception occurs.
     * @exception FinderException is thrown for application level errors.
     */
    public Collection findByRefConfigSetID(Integer refConfigSetID)
             throws FinderException, RemoteException;


    /**
     * Standard EJB implementation
     *
     * @return a collection of DeviceGroup EJBObjects of the beans located.
     * @exception RemoteException is thrown if a System exception occurs.
     * @exception FinderException is thrown for application level errors.
     */
    public Collection findAll()
             throws FinderException, RemoteException;

    /**
     * finds all of the Device Groups with no parent.
     *
     * @return a Collection of DeviceGroup EJBObjects of the beans located.
     * @throws FinderException is thrown for application level errors.
     * @throws RemoteException is thrown if a System exception occurs.
     */
    public Collection findTopGroups()
            throws FinderException, RemoteException;

}

