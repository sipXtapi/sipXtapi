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
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

/**
 * Responsible for generating MAC_ADDRESS.d/phone.cfg
 */
public class PhoneConfiguration extends ConfigurationTemplate {

    public PhoneConfiguration(PolycomPhone phone, Endpoint endpoint) {
        super(phone, endpoint);
    }

    protected void addContext(VelocityContext context) {
        super.addContext(context);
        context.put("lines", getLines());
    }

    public Collection getLines() {
        PolycomPhone phone = getPhone();
        ArrayList linesSettings = new ArrayList(phone.getMaxLineCount());

        List lines = getEndpoint().getLines();
        int i = 0;
        for (; lines != null && i < lines.size(); i++) {
            Line line = (Line) lines.get(i);
            linesSettings.add(line.getSettings(phone));
        }

        // copy in blank registrations of all unused lines
        for (; i < phone.getMaxLineCount(); i++) {
            Line blank = new Line();
            blank.setPosition(i);
            Setting model = phone.getSettingModel(blank);
            linesSettings.add(new ValueStorage().decorate(model));
        }
        
        return linesSettings;
    }
}
