/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/user/ApplicationGroupAdvocate.java#4 $
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

import java.rmi.RemoteException;

import javax.ejb.EJBObject;

import org.jdom.Element;

import com.pingtel.pds.common.PDSException;

public interface ApplicationGroupAdvocate extends ApplicationGroupAdvocateBusiness, EJBObject { }