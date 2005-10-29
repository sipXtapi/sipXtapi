/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.alias;

import java.lang.reflect.Proxy;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;

import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.context.event.ContextRefreshedEvent;

// DO_NOW: Add the AliasOwner interface to all appropriate context beans and implement it.
// Change all alias-assigning code to check with the AliasManager before assigning an alias.
// TODO: Move extension-related classes into this package and move any bean definitions into this beans.xml.

/**
 * AliasManager: manages all SIP aliases
 */
public class AliasManager implements AliasOwner, ApplicationListener {
    public static final String CONTEXT_BEAN_NAME = "aliasManager";
    private Collection m_aliasOwners;
    private ListableBeanFactory m_beanFactory;
    
    /** Return true if the alias is in use by some AliasOwner */
    public boolean isAliasInUse(String alias) {
        boolean isInUse = false;
        for (Iterator iter = getAliasOwners().iterator(); iter.hasNext();) {
            AliasOwner owner = (AliasOwner) iter.next();
            if (owner.isAliasInUse(alias)) {
                isInUse = true;
                break;
            }
        }
        return isInUse;
    }

    /** Return all alias owners (excluding the AliasManager itself) */
    protected Collection getAliasOwners() {
        if (m_aliasOwners == null) {    // lazy initialization
            if (m_beanFactory == null) {
                throw new BeanInitializationException(getClass().getName() + " not initialized");
            }
            Map beanMap = m_beanFactory.getBeansOfType(AliasOwner.class, false, true);
            m_aliasOwners = new ArrayList(beanMap.size());
            // Collect all proxies
            for (Iterator i = beanMap.values().iterator(); i.hasNext();) {
                AliasOwner owner = (AliasOwner) i.next();
                // Only include beans created through Factories - need Hibernate support.
                // Exclude the AliasManager itself, or we'll get into infinite recursion
                // when calling the alias owners.
                if (owner instanceof Proxy && !(owner instanceof AliasManager)) {
                    m_aliasOwners.add(owner);
                }
            }
            
        }
        return m_aliasOwners;
    }

    /**
     * Updates reference to bean factory and cleans the cache of alias owners
     */
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ContextRefreshedEvent) {
            ContextRefreshedEvent cre = (ContextRefreshedEvent) event;
            m_beanFactory = cre.getApplicationContext();
            m_aliasOwners = null;
            
            // For debugging purposes, print out the location of the file log4j.properties
            // on the classpath, if it is there.  No particular reason for this code to be
            // here, but it needed to go somewhere.
            URL url = getClass().getResource("log4j.properties");
            if (url != null) {
                System.out.println("Found log4j.properties: " + url);
            }
            
        }
    }

}
