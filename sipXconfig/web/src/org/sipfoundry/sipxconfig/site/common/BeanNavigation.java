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
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

@ComponentClass(allowBody = true, allowInformalParameters = true)
public abstract class BeanNavigation<T extends BeanWithSettings> extends BaseComponent {
    
    @Parameter(required = true)
    public abstract T getBean();
    
    public abstract void setBean(T bean);
   
    @Parameter()
    public abstract Setting getActiveSetting();
}
