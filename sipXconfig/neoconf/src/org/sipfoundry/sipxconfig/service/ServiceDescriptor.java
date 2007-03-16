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
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.device.DeviceDescriptor;

public class ServiceDescriptor extends DeviceDescriptor {

    public ServiceDescriptor() {        
    }
    
    public ServiceDescriptor(String beanId) {
        setBeanId(beanId);
    }
    
    public ServiceDescriptor(String beanId, String modelId) {
        this(beanId);
        setModelId(modelId);
    }    
}
