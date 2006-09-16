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
package org.sipfoundry.sipxconfig.domain;

import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;

public interface DomainManager extends DialingRuleProvider {
    
    public static final String CONTEXT_BEAN_NAME = "domainManager";
    
    public Domain getDomain();
    
    public void saveDomain(Domain domain);

    public static class DomainNotInitializedException extends RuntimeException {
        DomainNotInitializedException() {
            super("System was not initialized properly");
        }
    }
}
