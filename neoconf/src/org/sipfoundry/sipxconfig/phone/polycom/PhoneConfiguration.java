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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingGroup;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

/**
 * Responsible for generating phone.cfg
 */
public class PhoneConfiguration extends ConfigurationTemplate {
    
    public PhoneConfiguration(PolycomPhone phone, Endpoint endpoint) {
        super(phone, endpoint);
    }

    public void addContext(VelocityContext context) {
        context.put("lines", getLines());
    }

    public Collection getLines() {
        PolycomPhone phone = getPhone();
        ArrayList linesSettings = new ArrayList(phone.getMaxLineCount());

        List lines = getEndpoint().getLines();
        int i = 0;
        for (; lines != null && i < lines.size(); i++) {
            Line line = (Line) lines.get(i);
            SettingGroup settings = line.getSettings(phone); 
            linesSettings.add(settings);
        }

        // copy in blank registrations of all unused lines
        Line blank = new Line();
        SettingGroup model = phone.getSettingModel(blank);
        for (; i < phone.getMaxLineCount(); i++) {
            linesSettings.add(model.getCopy(new ValueStorage()));
        }
        
        return linesSettings;
    }
}
