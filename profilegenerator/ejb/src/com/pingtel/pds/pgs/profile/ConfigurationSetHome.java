/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/profile/ConfigurationSetHome.java#4 $
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

import javax.ejb.CreateException;
import javax.ejb.EJBHome;
import javax.ejb.FinderException;

import com.pingtel.pds.pgs.phone.Device;
import com.pingtel.pds.pgs.phone.DeviceGroup;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserGroup;

/**
 *  Description of the Interface
 *
 *@author     ibutcher
 *@created    December 13, 2001
 */
public interface ConfigurationSetHome extends EJBHome {

    /**
     *  Description of the Method
     *
     *@param  refConfigSet         Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@param  lp                   Description of the Parameter
     *@param  xmlContent           Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  CreateException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public ConfigurationSet create(Integer refConfigSet,
            int profileType, Device lp,
            String xmlContent)
             throws CreateException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  refConfigSet         Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@param  user                 Description of the Parameter
     *@param  xmlContent           Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  CreateException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public ConfigurationSet create(Integer refConfigSet,
            int profileType,
            User user,
            String xmlContent)
             throws CreateException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  refConfigSet         Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@param  pg                   Description of the Parameter
     *@param  xmlContent           Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  CreateException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public ConfigurationSet create(Integer refConfigSet,
            int profileType, DeviceGroup pg,
            String xmlContent)
             throws CreateException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  refConfigSet         Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@param  ug                   Description of the Parameter
     *@param  xmlContent           Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  CreateException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public ConfigurationSet create(Integer refConfigSet,
            int profileType,
            UserGroup ug,
            String xmlContent)
             throws CreateException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  pk                   Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public ConfigurationSet findByPrimaryKey(Integer pk)
             throws FinderException, RemoteException;


    /**
     *  Custom finder to find all Configuration Sets for a User Implemented by
     *  container uses JAWS XML for SQL
     *
     *@param  userID               Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public Collection findByUserIDAndProfileType(String userID,
            int profileType)
             throws FinderException, RemoteException;



    /**
     *  Custom finder to find all Configuration Sets for a LogicalPhone
     *  Implemented by container uses JAWS XML for SQL
     *
     *@param  logicalPhone         Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public Collection findByLogicalPhoneID(Integer logicalPhone)
             throws FinderException, RemoteException;


    /**
     *  Custom finder to find all Configuration Sets for a DeviceGroup
     *  Implemented by container uses JAWS XML for SQL
     *
     *@param  deviceGroup          Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public Collection findByDeviceGroupID(Integer deviceGroup)
             throws FinderException, RemoteException;


    /**
     *  Custom finder to find all Configuration Sets for a UserGroup
     *
     *@param  userGroupID          Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public Collection findByUserGroupIDAndProfileType(Integer userGroupID,
            int profileType)
             throws FinderException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  userID               Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public Collection findByUserID(String userID)
             throws FinderException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  userID               Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public Collection findByUserGroupID(Integer userID)
             throws FinderException, RemoteException;
}
