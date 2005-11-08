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
package org.sipfoundry.sipxconfig.api;

import java.beans.PropertyDescriptor;
import java.lang.reflect.Array;
import java.lang.reflect.InvocationTargetException;
import java.util.Set;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.beanutils.PropertyUtils;

public final class ApiBeanUtil {

    private ApiBeanUtil() {
    }
    
    public static void setProperties(Object otherObject, Property[] properties) {
        for (int i = 0; i < properties.length; i++) {
            try {
                BeanUtils.setProperty(otherObject, properties[i].getProperty(), properties[i].getValue());
            } catch (IllegalAccessException iae) {
                throw new RuntimeException(iae);
            } catch (InvocationTargetException ite) {
                // TODO: possible property spelling error, throw better error here
                throw new RuntimeException(ite);
            }
        }        
    }
    
    public static Object[] toApiArray(ApiBeanBuilder builder, Object[] fromOther, Class toApiClass) {
        Object[] apiObject = (Object[]) Array.newInstance(toApiClass, fromOther.length);
        for (int i = 0; i < fromOther.length; i++) {
            try {
                apiObject[i] = toApiClass.newInstance();
            } catch (InstantiationException impossible1) {
                wrapImpossibleException(impossible1);
            } catch (IllegalAccessException impossible2) {
                wrapImpossibleException(impossible2);
            }
            builder.toApi(apiObject[i], fromOther[i]);
        }
        
        return apiObject;
        
    }
    
    static void wrapImpossibleException(Exception e) {
        throw new RuntimeException("Impossible exception made possible", e);        
    }
    
    public static void copyProperties(Object from, Object to, Set ignoreList) {        
        try {
            PropertyDescriptor[] origDescriptors =
                PropertyUtils.getPropertyDescriptors(from);
            for (int i = 0; i < origDescriptors.length; i++) {
                String name = origDescriptors[i].getName();
                if ("class".equals(name)) {
                    continue; // No point in trying to set an object's class
                }
                boolean ignore = (ignoreList != null && ignoreList.contains(name)); 
                if (!ignore && PropertyUtils.isReadable(from, name)
                        && PropertyUtils.isWriteable(to, name)) {
                    try {
                        Object value = PropertyUtils.getSimpleProperty(from, name);
                        BeanUtils.copyProperty(to, name, value);
                    } catch (NoSuchMethodException impossible) {
                        throw new RuntimeException(impossible);
                    }
                }
            }
        } catch (IllegalAccessException iae) {
            throw new RuntimeException(iae);
        } catch (InvocationTargetException ite) {
            throw new RuntimeException(ite);
        }
    }
}
