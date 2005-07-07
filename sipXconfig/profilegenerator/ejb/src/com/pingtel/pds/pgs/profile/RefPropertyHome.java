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

import javax.ejb.CreateException;
import javax.ejb.EJBHome;
import javax.ejb.FinderException;

public interface RefPropertyHome extends EJBHome {

    public RefProperty create ( String name,
                                int profileType,
                                String code,
                                String content )
        throws CreateException, RemoteException;

    public RefProperty findByPrimaryKey ( Integer pk )
        throws FinderException, RemoteException;

    public Collection findAll()
        throws FinderException, RemoteException;

    public Collection findByProfileType ( int profileType )
        throws FinderException, RemoteException;

    public Collection findByRefConfigurationSetID ( Integer refConfigurationSetID )
        throws FinderException, RemoteException;

    public Collection findByCode ( String code )
        throws FinderException, RemoteException;

}
