/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/util/LibraryConfig.java#4 $
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

import java.util.ResourceBundle;

public class LibraryConfig {

    public static final String PRP_FILE 
        = "book/libdefaults";
    
    private static boolean fieldPlacementInUse = false;        
    private static boolean jdbcFieldPlacementInUse = false;
    private static LibraryConfig lc 
        = new LibraryConfig();
                        
    public static boolean isFieldPlacementInUse()
    {
        return fieldPlacementInUse;
    }
    
    public static boolean isJDBCFieldPlacementInUse()
    {
        return jdbcFieldPlacementInUse;
    }
    
    public LibraryConfig()
    {
        try {
            ResourceBundle props = 
                ResourceBundle.getBundle(PRP_FILE);
              
            fieldPlacementInUse = getBooleanPrp(props, 
                                                "fieldPlacementInUse", 
                                                false);
                                                
            jdbcFieldPlacementInUse = getBooleanPrp(props, 
                                                    "jdbcFieldPlacementInUse", 
                                                    false);
        } catch(Throwable t) {
            // We do not care ...
        }
    }
    
    protected boolean getBooleanPrp(ResourceBundle p, 
                                    String name,
                                    boolean defValue)
    {
        String v = p.getString(name);
        if(null != v) {
            v = v.trim();
            if(v.length() > 0) {
                return new Boolean(v).booleanValue();
            }
        }

        return defValue;            
    }
    
    protected int getIntPrp(ResourceBundle p, 
                            String name,
                            int defValue)
    {
        String v = p.getString(name);
        if(null != v) {
            v = v.trim();
            if(v.length() > 0) {
                return new Integer(v).intValue();
            }
        }

        return defValue;            
    }    
    
    protected String getStringPrp(ResourceBundle p, 
                                  String name,
                                  String defValue)
    {
        String v = p.getString(name);
        if(null != v) {
            v = v.trim();
            if(v.length() > 0) {
                return v;
            }
        }

        return defValue;            
    }       
}