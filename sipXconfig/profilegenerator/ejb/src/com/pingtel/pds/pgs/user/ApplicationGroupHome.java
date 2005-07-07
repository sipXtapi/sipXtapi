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

public interface ApplicationGroupHome extends EJBHome {

    public ApplicationGroup create (    String name,
                                        Integer organizationID )
        throws CreateException, RemoteException, PDSException;

    public Collection findByOrganizationID ( Integer organizationID )
        throws FinderException, RemoteException;

    public ApplicationGroup findByPrimaryKey ( Integer id )
        throws FinderException, RemoteException;

    public Collection findByUserID ( String userID )
        throws FinderException, RemoteException;

    public Collection findByUserGroupID ( Integer userGroupID )
        throws FinderException, RemoteException;

    public Collection findByApplicationID ( Integer applicationID )
        throws FinderException, RemoteException;

    public Collection findByName ( String name )
        throws FinderException, RemoteException;

    public Collection findAll ( ) throws FinderException, RemoteException;
}
