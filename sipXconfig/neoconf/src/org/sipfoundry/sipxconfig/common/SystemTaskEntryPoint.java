/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;


/**
 * For database tasks typically initiated from command line, this will 
 * get system started  
 */
public interface SystemTaskEntryPoint {
    
    /**
     * First argument is the beanId, remaining args where passed in from 
     * database.xml file
     */
    public void runSystemTask(String[] args);
}
