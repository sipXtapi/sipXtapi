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
package org.sipfoundry.sipxconfig.setting;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang.StringUtils;

/**
 * Treat any bean as a value storage, mapping properties to setting
 * paths.
 * 
 * Methods should return external representations, e.g. booleans instead
 * of "0" or "1" for example. 
 * 
 * Example:
 * 
 *   public static class MyDefaults {
 *    
 *     @SettingEntry(path = "hat/color")
 *     public String getHatColor() {
 *       return "red";
 *     }
 *   }
 *   
 *   public void initialize() {
 *     addDefaultBeanValueStorage(new MyDefaults());
 *   }
 */
public class BeanValueStorage implements SettingValueHandler {
    private Object m_bean;
    private Map<String, Method> m_properties = new HashMap();
    
    public BeanValueStorage(Object bean) {
        m_bean = bean;
        for (Method m : m_bean.getClass().getMethods()) {
            if (m.isAnnotationPresent(SettingEntry.class)) {
                SettingEntry entry = m.getAnnotation(SettingEntry.class);
                if (!StringUtils.isBlank(entry.path())) {
                    m_properties.put(entry.path(), m);
                } else {
                    for (String s : entry.paths()) {                        
                        m_properties.put(s, m);
                    }
                }
            }
        }        
    }

    public SettingValue2 getSettingValue(Setting setting) {
        SettingValue2 value = null;
        
        String path = setting.getPath();
        Method m = m_properties.get(path);
        if (m != null) {
            Object o;
            try {                
                // should be getter (although not strictly nec), with no args
                o = m.invoke(m_bean, new Object[0]);
//                String svalue = o == null ? null : o.toString();                
                
                String svalue = setting.getType().convertToStringValue(o);
                
                value = new SettingValueImpl(svalue);                
            } catch (IllegalArgumentException e) {
                throw new RuntimeException(e);
            } catch (IllegalAccessException e) {
                throw new RuntimeException(e);
            } catch (InvocationTargetException e) {
                throw new RuntimeException(e);
            }
        }

        return value;
    }

}
