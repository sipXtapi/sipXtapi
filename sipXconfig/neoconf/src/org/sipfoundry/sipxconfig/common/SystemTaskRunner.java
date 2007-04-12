/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.context.ApplicationContext;
import org.springframework.context.access.ContextSingletonBeanFactoryLocator;

/**
 * Triggers system to start up, then find a requested bean, then runs it.
 */
public class SystemTaskRunner {

    public static void main(String[] args) {
        if (args == null || args.length == 0) {
            throw new IllegalArgumentException("bean to run is required as first argument");
        }

        new SystemTaskRunner().runMain(args);
    }
    
    void runMain(String[] args) {
        BeanFactoryLocator bfl = ContextSingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bfr = bfl.useBeanFactory("servicelayer-context");
        ApplicationContext app = (ApplicationContext) bfr.getFactory();
        SystemTaskEntryPoint task = (SystemTaskEntryPoint) app.getBean(args[0]);
        task.runSystemTask(args);
    }
}
