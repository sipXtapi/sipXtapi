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
package com.pingtel.pds.common;

import java.text.MessageFormat;
import java.util.Hashtable;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

/**
 */
public class LocalStrings {
    private static final String LSTRING_FILE_NAME = "LocalStrings";
    private static Hashtable sPackage2Strings = new Hashtable();    

    /**
     * 
     * @param _packageName
     * 
     * @return 
     */
    public static LocalStrings getLocalStringsForPackage (String _packageName) {
        return getLocalStringsForPackage(_packageName, null);
    }

    /**
     * 
     * @param _packageName
     * @param _l
     * 
     * @return 
     */
    public static LocalStrings getLocalStringsForPackage(String _packageName, Locale _l) {
        LocalStrings rc = (LocalStrings) sPackage2Strings.get(_packageName);
        if (null == rc) {
            ResourceBundle rb = 
                ResourceBundle.getBundle(
                    _packageName + "." + LSTRING_FILE_NAME);
            if (null != rb) {
                rc = new LocalStrings(rb);
                sPackage2Strings.put(_packageName, rc);
            }
        }

        return rc;
    }

    /**
     * 
     * @param _claz
     * 
     * @return 
     */
    public static LocalStrings getLocalStrings(Class _claz) {
        return getLocalStrings(_claz, null);
    }

    /**
     * 
     * @param _claz
     * @param _l
     * 
     * @return 
     */
    public static LocalStrings getLocalStrings(Class _claz, Locale _l) {
        String packageName = "";
        String className = _claz.getName();        
        int locOfDoc = className.lastIndexOf('.');

        if (locOfDoc > 0) {
            packageName = className.substring(0, locOfDoc);
        }

        return getLocalStringsForPackage(packageName, _l);
    }

    protected ResourceBundle mRb;

    /**
     * 
     * @param _rb
     */
    public LocalStrings(ResourceBundle _rb) {
        mRb = _rb;
    }

    /**
     * 
     * @param _id
     * 
     * @return 
     */
    public String getStr(String _id) {
        return mRb.getString(_id);
    }
    
    public String getStr(String _id, String defaultString) {
        try {
        	return getStr(_id);
        }
        catch (MissingResourceException useDefault)
		{		
		}
        return defaultString;
    }

    /**
     * 
     * @param _id
     * @param _params
     * 
     * @return 
     */
    public String getFormated(String _id, Object []_params) {
        return MessageFormat.format(getStr(_id), _params);
    }

    /**
     * 
     * @param _id
     * @param _param
     * 
     * @return 
     */
    public String getFormated(String _id, Object _param) {
        return getFormated(_id, 
                           new Object[] {_param});
    }

    /**
     * 
     * @param _id
     * @param _param1
     * @param _param2
     * 
     * @return 
     */
    public String getFormated(String _id, Object _param1, Object _param2) {
        return getFormated(_id, new Object[] {_param1, _param2});
    }

    /**
     * 
     * @param _id
     * @param _param1
     * @param _param2
     * @param _param3
     * 
     * @return 
     */
    public String getFormated(String _id, Object _param1, Object _param2, Object _param3) {
        return getFormated(_id, new Object[] {_param1, _param2, _param3});
    }

    /**
     * 
     * @param _id
     * @param _param1
     * @param _param2
     * @param _param3
     * @param _param4
     * 
     * @return 
     */
    public String getFormated(String _id, Object _param1, Object _param2, Object _param3, Object _param4) {
        return getFormated(_id, new Object[] {_param1, _param2, _param3, _param4});
    }    
}
