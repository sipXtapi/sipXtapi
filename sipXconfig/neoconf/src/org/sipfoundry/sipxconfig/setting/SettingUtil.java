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

public final class SettingUtil {
    private SettingUtil() {
    }

    /**
     * Non inclusive (e.g. will never include root setting in collection) User a filter to
     * generate a collection. Runner will naturally recurse tree up to filter to accept/reject
     * settings. For chaining filters, try creating a composite filter instead for running
     * collections back into this runner.
     * 
     * <pre>
     *           Example:
     *             class MyClass {
     *           
     *                  private static final SettingFilter MY_SETTING = new SettingFilter() {
     *                      public boolean acceptSetting(Setting root_, Setting setting) {
     *                          return setting.getName().equals(&quot;mysetting&quot;);
     *                      }
     *                  };
     *                  
     *                 public Collection getMySettings(Setting settings) {
     *                       return SettingUtil.filter(MY_SETTINGS, settings);
     *                 }
     *             }
     * </pre>
     */
    public static Collection<Setting> filter(SettingFilter filter, Setting root) {
        FilterRunner runner = new FilterRunner(filter, root);
        for (Setting s : root.getValues()) {
            s.acceptVisitor(runner);
        }
        return runner.m_collection;
    }

    /**
     * When you do not have access to the root node, but you know a setting is a child to another,
     * you can retrieve that child
     * 
     * <pre>
     *           Example:
     *             root path:     /
     *             setting path:  /a/b
     *             fullPath:      /a/b/c/d
     *             
     *             return node for /a/b/c/d
     * </pre>
     * 
     * @param setting a setting somewhere in the path of fullPath
     * @param fullPath
     * @return child for full path
     */
    // public static Setting getSettingFromNode(Setting setting, String fullPath) {
    // String prefix = setting.getPath() + Setting.PATH_DELIM;
    // if (!fullPath.startsWith(prefix)) {
    // return null;
    // }
    //        
    // String path = fullPath.substring(prefix.length());
    // Setting child = setting.getSetting(path);
    //        
    // return child;
    // }
    /**
     * If a setting set is advanced, then all it's children can be considered advanced. USE CASE :
     * XCF-751
     * 
     * @param node parent to some level of setting
     * @param setting descendant of parent
     * @return true if any node is advanced including node itself
     */
    public static boolean isAdvancedIncludingParents(Setting node, Setting setting) {
        Setting s = setting;
        while (s != null && s != node) {
            if (s.isAdvanced()) {
                return true;
            }
            s = s.getParent();
        }
        return node.isAdvanced();
    }

    public static boolean isLeaf(Setting setting) {
        return setting.getValues().isEmpty();
    }

    public static final String subpath(String path, int start) {
        if (path == null) {
            return null;
        }

        int pos = 0;
        for (int i = 0; i < start && pos >= 0; i++, pos++) {
            pos = path.indexOf(Setting.PATH_DELIM, pos);
        }

        return pos >= 0 ? path.substring(pos) : path;
    }

    static class FilterRunner implements SettingVisitor {
        private Collection<Setting> m_collection = new ArrayList<Setting>();

        private SettingFilter m_filter;

        private Setting m_root;

        FilterRunner(SettingFilter filter, Setting root) {
            m_filter = filter;
            m_root = root;
        }

        public void visitSetting(Setting setting) {
            if (m_filter.acceptSetting(m_root, setting)) {
                m_collection.add(setting);
            }
        }

        public boolean visitSettingGroup(Setting settingGroup) {
            visitSetting(settingGroup);
            return true;
        }
    }

    /**
     * HACK UNTIL DECORATORS ARE REMOVED
     */
    public static SettingImpl getSettingImpl(Setting s) {
        if (s instanceof SettingImpl) {
            return (SettingImpl) s;
        }

        throw new RuntimeException("Unknown Setting Type " + s.getClass().getName());
    }

}
