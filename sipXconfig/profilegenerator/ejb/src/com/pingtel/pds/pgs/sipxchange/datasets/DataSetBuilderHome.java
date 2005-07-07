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

package com.pingtel.pds.pgs.sipxchange.datasets;

import com.pingtel.pds.pgs.sipxchange.datasets.DataSetBuilder;

import java.rmi.RemoteException;

import javax.ejb.CreateException;
import javax.ejb.EJBHome;



public interface DataSetBuilderHome extends EJBHome {

    public DataSetBuilder create()
             throws CreateException, RemoteException;
}
