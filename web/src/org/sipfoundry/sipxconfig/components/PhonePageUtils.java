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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneDao;
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
        BeanFactory bf = TapestryUtils.getBeanFactory(cycle);
        return (PhoneContext) bf.getBean(PhoneContext.CONTEXT_BEAN_NAME);
    }
    
    /**
     * Convienence method to phone data access object
     */
    public static final PhoneDao getPhoneDao(IRequestCycle cycle) {
        return getPhoneContext(cycle).getPhoneDao();
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
