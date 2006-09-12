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

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.sipfoundry.sipxconfig.common.InitializationTask;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

/**
 * When system first starts up, create initial domain object w/default value(s)
 */
public class DomainInitializer implements ApplicationListener {
    private DomainManager m_domainManager;
    private String m_initialDomain;

    public void setInitialDomain(String initialDomain) {
        m_initialDomain = initialDomain;
    }

    public DomainManager getDomainManager() {
        return m_domainManager;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof InitializationTask) {
            InitializationTask dbEvent = (InitializationTask) event;
            if (dbEvent.getTask().equals("initialize-domain")) {
                initializeDomain();
            }
        }
    }
    
    void initializeDomain() {
        Domain domain = createDomain();
        m_domainManager.saveDomain(domain);        
    }
    
    public Domain createDomain() {
        Domain d = new Domain();
        d.setName(getInitialDomainName());
        return d;        
    }
    
    public String getInitialDomainName() {
        if (m_initialDomain != null) {
            return m_initialDomain;
        }
        
        try {
            InetAddress addr = InetAddress.getLocalHost();
            m_initialDomain = addr.getHostName();
        } catch (UnknownHostException e) {
            throw new RuntimeException("Could not determine hostname", e);
        }
        return m_initialDomain;
    }
}
