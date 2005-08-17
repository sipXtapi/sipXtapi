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
package org.sipfoundry.sipxconfig.setting;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

/**
 * User a filter to generate a collection. Runner will naturally recurse tree
 * up to filter to accept/reject settings.  For chaining filters, try creating
 * a composite filter instead for running collections back into this runner.
 * <pre>
 * Example:
 *   class MyClass {
 * 
 *        private static final SettingFilter MY_SETTING = new SettingFilter() {
 *            public boolean acceptSetting(Setting root_, Setting setting) {
 *                return setting.getName().equals("mysetting");
 *            }
 *        };
 *        
 *       public Collection getMySettings(Setting settings) {
 *             return FilterRunner().filter(MY_SETTINGS, settings);
 *       }
 *   }
 * </pre>
 */
public final class FilterRunner implements SettingVisitor {
        
    private Collection m_collection = new ArrayList();
    
    private SettingFilter m_filter;
    
    private Setting m_root;
    
    private FilterRunner(SettingFilter filter, Setting root) {
        m_filter = filter;
        m_root = root;
    }
    
    /**
     * Non inclusive (e.g. will never include root setting in collection)  
     */
    public static Collection filter(SettingFilter filter, Setting root) {
        FilterRunner runner = new FilterRunner(filter, root);
        Iterator i = root.getValues().iterator();
        while (i.hasNext()) {
            Setting s = (Setting) i.next(); 
            s.acceptVisitor(runner);
        }
        return runner.m_collection;
    }    

    public void visitSetting(Setting setting) {        
        if (m_filter.acceptSetting(m_root, setting)) {
            m_collection.add(setting);
        }
    }

    public void visitSettingGroup(Setting settingGroup) {
        visitSetting(settingGroup);
    }
}
