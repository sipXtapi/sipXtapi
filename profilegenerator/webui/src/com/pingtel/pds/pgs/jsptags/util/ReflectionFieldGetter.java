/*
 * 
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

import java.beans.IntrospectionException;
import java.lang.reflect.InvocationTargetException;

import com.pingtel.pds.common.LocalStrings;

public class ReflectionFieldGetter 
    implements FieldGetter {
    
    static LocalStrings ls =
        LocalStrings.getLocalStrings(ReflectionFieldGetter.class);
    
    protected Object o;
    
    public void setObject(Object o)
        throws IllegalArgumentException
    {
        this.o = o;
    }
    
    public Object getField(String fieldName)
        throws IllegalAccessException
    {
        try {
            return BeanUtil.getObjectPropertyValue(o,
                                                   fieldName,
                                                   null);
        } catch(InvocationTargetException ex) {
            // Fall through
        } catch(IllegalAccessException ex) {
            // Fall through
        } catch(IntrospectionException ex) {
            // Fall through
        } catch(NoSuchMethodException ex) {            
            // Fall through
        }
        
        throw new IllegalAccessException(ls.getFormated(Constants.CANNOT_ACCESS_FIELD,
                                                        fieldName));
    }
}