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

import org.sipfoundry.sipxconfig.common.InitializationTask;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.context.ApplicationContext;
import org.springframework.context.access.ContextSingletonBeanFactoryLocator;

/**
 * Iterates through all beans in Spring context and calls the beans that implement Patch interface
 * with opportunity to apply a db patch.
 */
public class DataInitializer {
    
    public static void main(String[] args) {
        new DataInitializer().runMain();        
        if (args.length == 0 || !"noexit".equals(args[0])) {
            System.exit(0);
        }
    }
    
    void runMain() {
        BeanFactoryLocator bfl = ContextSingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bfr = bfl.useBeanFactory("servicelayer-context");
        ApplicationContext app = (ApplicationContext) bfr.getFactory();
        AdminContext adminContext = (AdminContext) app.getBean(AdminContext.CONTEXT_BEAN_NAME);
        String[] tasks = adminContext.getInitializationTasks();
        for (int i = 0; i < tasks.length; i++) {
            initializeData(tasks[i], app, adminContext);
        }
    }
    
    void initializeData(String task, ApplicationContext app, AdminContext admin) {
        InitializationTask event = new InitializationTask(task);
        app.publishEvent(event);
        admin.deleteInitializationTask(task);            
    }
}
