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
package org.sipfoundry.sipxconfig.admin;

import java.util.Iterator;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.Patch;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.context.access.ContextSingletonBeanFactoryLocator;

/**
 * Iterates through all beans in Spring context and calls the beans that implement Patch interface
 * with opportunity to apply a db patch.
 */
public class PatchRunner {
    
    public static void main(String[] args) {
        new PatchRunner().runMain(args);
        System.exit(0);
    }
    
    void runMain(String[] args) {
        String patchName = getPatchName(args);
        BeanFactoryLocator bfl = ContextSingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bfr = bfl.useBeanFactory("servicelayer-context");
        ListableBeanFactory factory = (ListableBeanFactory) bfr.getFactory();
        applyPatch(patchName, factory);
    }
    
    void applyPatch(String patchName, ListableBeanFactory factory) {
        AdminContext adminContext = (AdminContext) factory.getBean(AdminContext.CONTEXT_BEAN_NAME);
        Map patchBeans = factory.getBeansOfType(Patch.class);
        Iterator patches = patchBeans.values().iterator();
        while (patches.hasNext()) {
            Patch patch = (Patch) patches.next();
            if (patch.applyPatch(patchName)) {
                adminContext.setPatchApplied(patchName);
                break;
            }
        }
    }
    
    
    String getPatchName(String[] args) {
        if (args == null || args.length <= 0 || args[0] == null) {
            throw new IllegalArgumentException("first argument is name of bean to be run");
        }
        return args[0];
    }
}
