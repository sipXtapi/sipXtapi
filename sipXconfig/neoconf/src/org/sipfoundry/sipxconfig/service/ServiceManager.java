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

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.DataObjectSource;



public interface ServiceManager extends DataObjectSource<ConfiguredService> {
    public static final String CONTEXT_ID = "serviceManager";
    
    public ConfiguredService newService(ServiceDescriptor descriptor);

    public ConfiguredService loadService(Integer serviceId);
    
    public void saveService(ConfiguredService service);

    public void deleteService(ConfiguredService service);

    public Collection<ConfiguredService> getServices();

    public Collection<ConfiguredService> getServicesByType(ServiceDescriptor descriptor);
    
    public void clear();
}
