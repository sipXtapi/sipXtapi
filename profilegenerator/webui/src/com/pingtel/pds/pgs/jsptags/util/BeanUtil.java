/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/util/BeanUtil.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
package com.pingtel.pds.pgs.jsptags.util;

import java.util.Hashtable;

import java.beans.BeanInfo;
import java.beans.Introspector;
import java.beans.PropertyDescriptor;
import java.beans.IndexedPropertyDescriptor;
import java.beans.IntrospectionException;

import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

public class BeanUtil {
    static LocalStrings ls =
                LocalStrings.getLocalStrings(BeanUtil.class);

    public static final Object []sNoParams = new Object[0];
    public static Hashtable sGetPropToMethod = new Hashtable(100);
    public static Hashtable sSetPropToMethod = new Hashtable(100);

    public static Object
        getObjectPropertyValue(Object obj,
                               String propName,
                               Object index)
            throws InvocationTargetException,
                   IllegalAccessException,
                   IntrospectionException,
                   NoSuchMethodException
    {
        Method m = getGetPropertyMethod(obj,
                                        propName,
                                        null == index ?
                                        null: index.getClass());
        if(null == index) {
            return m.invoke(obj, sNoParams);
        } else {
            Object []params = new Object[1];
            params[0] = index;
            return m.invoke(obj, params);
        }
    }

    public static Method
        getGetPropertyMethod(Object obj,
                             String propName,
                             Class  paramClass)
            throws IntrospectionException,
                   NoSuchMethodException
    {
        Class     oClass = obj.getClass();
        MethodKey key = new MethodKey(propName,
                                      oClass,
                                      paramClass);
        Method rc = (Method)sGetPropToMethod.get(key);
        if(rc != null) {
            return rc;
        }

        BeanInfo           info = Introspector.getBeanInfo(oClass);
        PropertyDescriptor[] pd = info.getPropertyDescriptors();

        if(null != pd) {
            for(int i = 0; i < pd.length;  i++) {
                if(pd[i] instanceof IndexedPropertyDescriptor) {
                    if(null == paramClass ||
                       !propName.equals(pd[i].getName())) {
                        continue;
                    }
                    IndexedPropertyDescriptor ipd =
                        (IndexedPropertyDescriptor)pd[i];
                    Method m = ipd.getIndexedReadMethod();
                    if(null == m) {
                        continue;
                    }
                    Class[]params = m.getParameterTypes();
                    if((1 == params.length) &&
                        params[0].equals(paramClass)) {
                        rc = m;
                        break;
                    }
                } else {
                    if(null != paramClass ||
                       !propName.equals(pd[i].getName())) {
                        continue;
                    }
                    rc = pd[i].getReadMethod();
                    break;
                }
            }
        }

        /*
         * Looking into the propety descriptor failed. It can be becuase
         * that there are two properties with the same name and different
         * indices (for example the "string" property in a ResultSet)
         *
         * Look into the methods.
         */
        if(null == rc) {
            StringBuffer methodName = new StringBuffer();
            methodName.append("get");
            methodName.append(propName.substring(0,1).toUpperCase());
            methodName.append(propName.substring(1));

            if(null == paramClass) {
                rc = oClass.getMethod(methodName.toString(),
                                      new Class[0]);
            } else {
                rc = oClass.getMethod(methodName.toString(),
                                      new Class[] {paramClass});
            }
        }

        if(null == rc) {
            throw new NoSuchMethodException(ls.getFormated(Constants.NO_GET_PROPERTY_METHOD,
                                                           propName,
                                                           null == paramClass ? Void.class : paramClass));
        }
        sGetPropToMethod.put(key, rc);
        return rc;
    }

    /*
     * We are not dealing with indexed properties.
     *
     */
    public static Method
        getSetPropertyMethod(Object obj,
                             String propName)
            throws IntrospectionException,
                   NoSuchMethodException
    {
        Class     oClass = obj.getClass();
        MethodKey key = new MethodKey(propName,
                                      oClass,
                                      null);
        Method rc = (Method)sSetPropToMethod.get(key);
        if(rc != null) {
            return rc;
        }

        BeanInfo             info = Introspector.getBeanInfo(oClass);
        PropertyDescriptor[] pd   = info.getPropertyDescriptors();

        if(null != pd) {
            for(int i = 0; i < pd.length;  i++) {
                if(propName.equals(pd[i].getName())) {
                    if(!(pd[i] instanceof IndexedPropertyDescriptor)) {
                        Method m = pd[i].getWriteMethod();
                        if(null == m) {
                            continue;
                        }
                        Class[]params = m.getParameterTypes();
                        if(1 == params.length) {
                            rc = m;
                            break;
                        }
                    }
                }
            }
        }

        if(null == rc) {
            throw new NoSuchMethodException(ls.getFormated(Constants.NO_SET_PROPERTY_METHOD,
                                                                 propName));
        }
        sSetPropToMethod.put(key, rc);
        return rc;
    }

    /*
     * We are not dealing with indexed properties.
     *
     *
    public static Method
        getSetPropertyMethod(Class claz,
                             String propName)
            throws IntrospectionException,
                   NoSuchMethodException
    {
        Method rc                 = null;
        BeanInfo             info = Introspector.getBeanInfo(claz);
        PropertyDescriptor[] pd   = info.getPropertyDescriptors();

        if(null != pd) {
            for(int i = 0; i < pd.length;  i++) {
                if(propName.equals(pd[i].getName()) &&
                   !(pd[i] instanceof IndexedPropertyDescriptor)) {
                    Method m = pd[i].getWriteMethod();
                    if(null == m) {
                        continue;
                    }
                    Class[]params = m.getParameterTypes();
                    if(1 == params.length) {
                        rc = m;
                        break;
                    }
                }
            }
        }

        if(null == rc) {
            throw new NoSuchMethodException();
        }

        return rc;
    }*/
}