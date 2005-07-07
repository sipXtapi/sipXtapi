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

import org.sipfoundry.sipxconfig.common.DataCollectionItem;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Line on a phone
 */
public interface Line extends DataCollectionItem {
    
    public Phone getPhone();
    
    public void setPhone(Phone phone);
    
    public void setLineData(LineData meta);

    public LineData getLineData();

    public Setting getSettings();
    
    public Setting getSettingModel();
    
    public String getUri();
    
    public void setUri(String uri);

    public Object getAdapter(Class adatper);
    
    /** implementation need to declare they implement PrimaryKeySource */
    public Object getPrimaryKey();
}
