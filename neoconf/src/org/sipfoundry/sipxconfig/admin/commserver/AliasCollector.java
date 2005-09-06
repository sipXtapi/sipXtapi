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
package org.sipfoundry.sipxconfig.admin.commserver;

import java.lang.reflect.Proxy;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;

import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.context.event.ContextRefreshedEvent;

/**
 * Finds all the beans that implement AliasProvider interface and retrieves the aliases for that
 * beans
 */
public class AliasCollector implements AliasProvider, ApplicationListener {

    private Collection m_aliasProviders;

    private ListableBeanFactory m_beanFactory;

    public Collection getAliasMappings() {
        Collection aliasProviders = getAliasProviders();
        Collection aliasMappings = new ArrayList();
        for (Iterator i = aliasProviders.iterator(); i.hasNext();) {
            AliasProvider provider = (AliasProvider) i.next();
            aliasMappings.addAll(provider.getAliasMappings());
        }

        return aliasMappings;
    }
    
    /**
     * Lazily creates the collection of beans that implement AliasProvider interface
     * 
     * 
     * @return cached or newly created listener collection
     */
    protected Collection getAliasProviders() {
        if (m_aliasProviders == null) {
            if (m_beanFactory == null) {
                throw new BeanInitializationException(getClass().getName() + " not initialized");
            }
            Map beanMap = m_beanFactory.getBeansOfType(AliasProvider.class, false, true);
            m_aliasProviders = new ArrayList(beanMap.size());
            for (Iterator i = beanMap.values().iterator(); i.hasNext();) {
                AliasProvider provider = (AliasProvider) i.next();
                // only include beans create through Factories - need hibernate support
                if (provider instanceof Proxy) {
                    m_aliasProviders.add(provider);
                }
            }            
        }
        return m_aliasProviders;
    }

    /**
     * Updates reference to bean factory and cleans the cache of listeners
     */
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ContextRefreshedEvent) {
            ContextRefreshedEvent cre = (ContextRefreshedEvent) event;
            m_beanFactory = cre.getApplicationContext();
            m_aliasProviders = null;
        }
    }

}
