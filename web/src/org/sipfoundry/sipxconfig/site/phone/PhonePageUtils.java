/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.SipxconfigEngine;
import org.springframework.beans.factory.BeanFactory;

/**
 * Helper functions for phone related Tapestry pages and components
 */
public final class PhonePageUtils {
    
    /**
     * restrict construction 
     */
    private PhonePageUtils() {
    }
    
    /**
     * Get the phone context from tapestry's cycle context
     */
    public static final PhoneContext getPhoneContext(IRequestCycle cycle) {
        PhoneContext context = (PhoneContext) cycle.getAttribute(PhoneContext.CONTEXT_BEAN_NAME);
        if (context == null) {
            SipxconfigEngine engine = (SipxconfigEngine) cycle.getEngine();
            BeanFactory bf = engine.getBeanFactory();
            context = (PhoneContext) bf.getBean(PhoneContext.CONTEXT_BEAN_NAME);
            cycle.setAttribute(PhoneContext.CONTEXT_BEAN_NAME, context);
        }
        
        return context;
        
    }    
}
