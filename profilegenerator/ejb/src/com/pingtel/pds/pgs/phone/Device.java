/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/phone/Device.java#4 $
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

import javax.ejb.EJBObject;

import com.pingtel.pds.common.PDSException;

/**
 *  LogicalPhone is the definition of the remote interface for the LogicalPhone
 *  EJ bean.
 *
 *@author     ibutcher
 *@created    December 13, 2001
 */
public interface Device extends EJBObject, DeviceBusiness {}
