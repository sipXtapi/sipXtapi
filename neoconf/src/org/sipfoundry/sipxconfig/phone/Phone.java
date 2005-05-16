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
package org.sipfoundry.sipxconfig.phone;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Implement this to add support for new devices to the system
 */
public interface Phone {
    
    public void setPhoneData(PhoneData meta);
    
    public PhoneData getPhoneData();
    
    /**
     * @return undecorated model - direct representation of XML model description
     */
    public Setting getSettingModel();

    /**
     * @return decorated model - use this to modify phone settings
     */
    public Setting getSettings();
    
    public void generateProfiles();
    
    public void restart();
        
    public List getLines();
    
    public void addLine(Line line);
    
    public Line getLine(int position);
    
    public Collection getDeletedLines();
    
    /** you must explicitly call addLine */
    public Line createLine(LineData lineMeta);

    /** implementation need to declare they implement PrimaryKeySource */
    public Object getPrimaryKey();
    
    public void setPhoneContext(PhoneContext phoneContext);
    
    public PhoneContext getPhoneContext();
}
