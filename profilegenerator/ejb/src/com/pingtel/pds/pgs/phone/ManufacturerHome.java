/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/phone/ManufacturerHome.java#4 $
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
 * LogicalPhoneHome is the Home interface for the Manufacturer EJ Bean
 *
 */
public interface ManufacturerHome extends EJBHome {

  /**
   *
   * @param       name the name that the Manufacturer will be known by.
   * @return      the EJBObject which is a proxy for the newly create
   * Manufacturer bean
   * @exception   CreateException thrown by the EJB Container if it is unable
   * to create the Manufacturer bean
   * @exception   RemoteException thrown if there is a system exception
  */
  public Manufacturer create ( String name )
    throws CreateException , RemoteException;

  /**
   *
   * @param       pk the primary key of the Manufacturer that you wish to
   * find in the EJB Container
   * @return      the EJBObject which is a proxy for the Manufacturer bean
   * that has been found
   * @exception   FinderException thrown by the EJB Container if it is unable
   * to find the Manufacturer.
   * @exception   RemoteException thrown if there is a system exception
  */
  public Manufacturer findByPrimaryKey ( Integer pk )
    throws FinderException, RemoteException;


    public Collection findByName ( String name )
        throws FinderException, RemoteException;


}