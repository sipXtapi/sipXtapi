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
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.springframework.beans.factory.BeanFactory;

/**
 * Helper functions for phone related Tapestry pages and components
 */
public final class PhonePageUtils {
    
    private static PhoneContext s_unittestPhoneContext;

    /**
     * restrict construction 
     */
    private PhonePageUtils() {
    }
    
    public static final void setUnittestPhoneContext(PhoneContext phoneContext) {
        s_unittestPhoneContext = phoneContext;
    }

    /**
     * Get the phone context from tapestry's cycle context
     */
    public static final PhoneContext getPhoneContext(IRequestCycle cycle) {
        PhoneContext phoneContext = s_unittestPhoneContext;
        if (phoneContext == null) {
            BeanFactory bf = TapestryUtils.getBeanFactory(cycle);
            phoneContext = (PhoneContext) bf.getBean(PhoneContext.CONTEXT_BEAN_NAME); 
        }

        return phoneContext;
    }
    
    /**
     * Get phone if pass as a parameter such as a DirectLink
     */
    public static final Phone getPhoneFromParameter(IRequestCycle cycle, int index) {
        Object[] params = cycle.getServiceParameters();
        Integer endpointId = (Integer) TapestryUtils.assertParameter(Integer.class, params, index);
        return getPhoneContext(cycle).getPhone(endpointId.intValue());
    }

}
