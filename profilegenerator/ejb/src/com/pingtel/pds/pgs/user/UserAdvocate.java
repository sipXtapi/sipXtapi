/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/user/UserAdvocate.java#4 $
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


import javax.ejb.EJBObject;

/**
 * Session Bean Facade for entitiy bean.
 * This is the Remote Interface
 */
public interface UserAdvocate extends EJBObject, UserAdvocateBusiness {}
