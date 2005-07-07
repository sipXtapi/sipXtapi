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



package com.pingtel.pds.pgs.user;

import com.pingtel.pds.common.PDSException;

import java.rmi.RemoteException;
import java.util.Collection;

import javax.ejb.CreateException;
import javax.ejb.EJBHome;
import javax.ejb.FinderException;


public interface UserGroupHome extends EJBHome {

    public UserGroup create (   String name,
                                Integer organizationID,
                                Integer refConfigSetID,
                                Integer parentGroupID )
        throws CreateException, RemoteException, PDSException;

    public UserGroup findByPrimaryKey ( Integer pk )
        throws FinderException, RemoteException;

    public Collection findByRefConfigSetID ( Integer refConfigSetID )
        throws FinderException, RemoteException;

    public Collection findByOrganizationID ( Integer organizationID )
        throws FinderException, RemoteException;

    public Collection findByParentID ( Integer parentID )
        throws FinderException, RemoteException;

    public Collection findByOrganizationIDAndName ( Integer organizationID, String name )
        throws FinderException, RemoteException;

    public Collection findAll ( ) throws FinderException, RemoteException;

    /**
     * findTopGroups finds all of the 'top' level groups in the config server's
     * database.   To be exact it returns all UserGroups whose parent ID is null.
     *
     * @return Collection of UserGroupHome objects.
     * @throws FinderException is thrown for application/container generated errors.
     * @throws RemoteException is thrown for system and remoting errors.
     */
    public Collection findTopGroups()
            throws FinderException, RemoteException;

}
