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
package org.sipfoundry.sipxconfig.site.service;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.service.ConfiguredService;
import org.sipfoundry.sipxconfig.service.ServiceManager;


public abstract class ListConfiguredServices extends BasePage {
    public static final String PAGE = "service/ListConfiguredServices";

    @InjectObject(value = "spring:serviceManager")
    public abstract ServiceManager getServiceManager();

    @Bean()
    public abstract SipxValidationDelegate getValidator();
    
    @Bean()
    public abstract SelectMap getSelections();

    public abstract ConfiguredService getCurrentRow();
    
    public abstract void setConverter(IPrimaryKeyConverter converter);
    
    public void pageBeginRender() {
        if (getRequestCycle().isRewinding()) {
            setConverter(new ObjectSourceDataSqueezer(getServiceManager(), ConfiguredService.class));
        }
    }
}
