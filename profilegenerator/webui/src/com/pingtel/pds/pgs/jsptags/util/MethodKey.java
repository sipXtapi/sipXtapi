/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/util/MethodKey.java#4 $
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

public class MethodKey {
    String mMethodName;
    Class  mObjectClass;
    Class  mParamClass;

    int    mHashCode;

    public MethodKey(String _methodName,
                     Class  _objectClass,
                     Class  _paramClass)
    {
        mMethodName     = _methodName;
        mObjectClass    = _objectClass;
        mParamClass     = _paramClass;

        mHashCode = _methodName.hashCode() +
                    _objectClass.hashCode() +
                    (null != _paramClass ? _paramClass.hashCode() : 0);
    }

    public boolean equals(Object obj)
    {
        if(obj instanceof MethodKey) {
            MethodKey other = (MethodKey)obj;
            if(mMethodName.equals(other.mMethodName) &&
               mObjectClass.equals(other.mObjectClass)) {
                if(null == mParamClass) {
                    if(null == other.mParamClass) {
                        return true;
                    }
                } else if(mParamClass.equals(other.mParamClass)) {
                    return true;
                }
            }
        }

        return false;
    }

    public int hashCode()
    {
        return mHashCode;
    }
}