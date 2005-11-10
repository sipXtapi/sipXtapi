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

import java.lang.reflect.Array;
import java.lang.reflect.InvocationTargetException;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.beanutils.PropertyUtils;

public final class ApiBeanUtil {
    
    private ApiBeanUtil() {
    }
    
    public static void toApiObject(ApiBeanBuilder builder, Object apiObject, Object myObject) {
        Set properties = getReadableProperties(apiObject);
        builder.toApiObject(apiObject, myObject, properties);
    }
    
    public static void toMyObject(ApiBeanBuilder builder, Object myObject, Object apiObject) {
        Set properties = getReadableProperties(apiObject);
        builder.toMyObject(myObject, apiObject, properties);
    }

    public static void setProperties(Object object, Property[] properties) {
        for (int i = 0; i < properties.length; i++) {
            setProperty(object, properties[i].getProperty(), properties[i].getValue());
        }        
    }
    
    public static Set getSpecfiedProperties(Property[] properties) {
        Set props = new HashSet(properties.length);
        for (int i = 0; i < properties.length; i++) {
            props.add(properties[i].getProperty());
        }
        return props;
    }
    
    public static Object[] newArray(Class elementClass, int size) {
        Object[] to = (Object[]) Array.newInstance(elementClass, size);
        for (int i = 0; i < size; i++) {
            try {                
                to[i] = elementClass.newInstance();
            } catch (InstantiationException impossible1) {
                wrapImpossibleException(impossible1);
            } catch (IllegalAccessException impossible2) {
                wrapImpossibleException(impossible2);
            }
        }
        
        return to;        
    }
    
    public static Object[] toApiArray(ApiBeanBuilder builder, Object[] myObjects, Class apiClass) {
        Object[] apiArray = ApiBeanUtil.newArray(apiClass, myObjects.length);
        if (myObjects.length == 0) {
            return apiArray; 
        }
        Set properties = ApiBeanUtil.getReadableProperties(apiArray[0]);
        for (int i = 0; i < myObjects.length; i++) {
            builder.toApiObject(apiArray[i], myObjects[i], properties);
        }
        return apiArray;
    }
    
    
    public static void wrapImpossibleException(Exception e) {
        throw new RuntimeException("Unexpected bean error", e);        
    }
    
    public static void wrapPropertyException(String property, Exception e) {
        throw new IllegalArgumentException("Error accessing property " + property + ": " + e.getMessage());                
    }
    
    public static Set getReadableProperties(Object o) {
        Set properties = null;
        try {
            Map desciption = BeanUtils.describe(o);
            desciption.remove("class");
            properties = desciption.keySet();
        } catch (IllegalAccessException e) {
            wrapImpossibleException(e);
        } catch (InvocationTargetException e) {
            wrapImpossibleException(e);
        } catch (NoSuchMethodException e) {
            wrapImpossibleException(e);
        }
        return properties;
    }
    
    public static void copyProperties(Object to, Object from, Set properties, Set ignoreList) {
        Iterator i = properties.iterator();
        while (i.hasNext()) {
            String name = (String) i.next();
            if (ignoreList != null && ignoreList.contains(name)) {
                continue;
            }
            try {
                Object value = PropertyUtils.getSimpleProperty(from, name);                
                BeanUtils.copyProperty(to, name, value);
            } catch (IllegalAccessException iae) {
                wrapPropertyException(name, iae);
                throw new RuntimeException(iae);
            } catch (InvocationTargetException ite) {
                wrapPropertyException(name, ite);
            } catch (NoSuchMethodException nsme) {
                wrapPropertyException(name, nsme);
            }
        }
    }

    public static void setProperty(Object o, String property, Object value) {
        try {
            BeanUtils.copyProperty(o, property, value);
        } catch (IllegalAccessException iae) {
            ApiBeanUtil.wrapPropertyException(property, iae);
        } catch (InvocationTargetException ite) {
            ApiBeanUtil.wrapPropertyException(property, ite);
        }
    }    
}
