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

import javax.ejb.EJBHome;
import javax.ejb.CreateException;
import java.rmi.RemoteException;
import javax.ejb.FinderException;
import java.util.Collection;

import com.pingtel.pds.common.PDSException;

public interface UserHome extends EJBHome {

    public User create ( Integer organizationID,
                         String firstName,
                         String lastName,
                         String password,
                         String displayID,
                         Integer refConfigSetID,
                         Integer userGroupID,
                         String extension )
        throws CreateException, PDSException, RemoteException;

    public User findByPrimaryKey ( String pk )
        throws FinderException, RemoteException;

    public Collection findByOrganizationID ( Integer organizationID )
        throws FinderException, RemoteException;

    public Collection findByRefConfigSetID ( Integer refConfigSetID )
        throws FinderException, RemoteException;

    public Collection findByUserGroupID ( Integer userGroupID )
        throws FinderException, RemoteException;

    public Collection findByDisplayID ( String displayID )
        throws FinderException, RemoteException;

    public Collection findAll ( ) throws FinderException, RemoteException;

    /** Custom finder to find only users with the ADMIN or SUPER role @JC */
    public Collection findAdministratorsByOrganizationID( Integer organizationID )
        throws FinderException, RemoteException;

    public Collection findByAlias( String alias )
        throws FinderException, RemoteException;

    public Collection findByExtension ( String extension )
        throws FinderException, RemoteException;

    public Collection findByDisplayIDOrExtension ( String extensionOrDisplayID )
        throws FinderException, RemoteException;
}
