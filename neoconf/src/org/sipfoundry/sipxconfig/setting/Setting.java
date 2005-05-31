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

import java.util.Collection;

import org.sipfoundry.sipxconfig.setting.type.SettingType;

/**
 * Base class for all items describing and using setting.  
 */    
public interface Setting {

    public static final String NULL_VALUE = new String();

    public Setting getParent();
    
    public void setParent(Setting setting);
    
    public String getPath(); 

    public Setting addSetting(Setting setting);

    public Setting getSetting(String name);      
    
    public String getLabel();

    public void setLabel(String label);

    public String getName();

    public void setName(String name);
    
    public String getProfileName();
    
    public void setProfileName(String profileName);

    /** 
     * what would value be if it wasn't set., most implementation this is your
     * the value from the setting you decorate.  NOTE: no setter because this
     * is a "computed" value based on chain of decorated setting values. 
     */
    public String getDefaultValue();

    public String getValue();

    public void setValue(String value);
    
    public SettingType getType();

    public void setType(SettingType type);
    
    public String getDescription();

    public void setDescription(String description);
    
    public Collection getValues();
    
    public void acceptVisitor(SettingVisitor visitor);
    
    public boolean isAdvanced();
    
    public void setAdvanced(boolean advanced);
    
    public Setting copy();
}


