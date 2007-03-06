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
import java.util.Map;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;

/**
 * Responsible for generating MAC_ADDRESS.d/phone.cfg
 */
public class PhoneConfiguration extends ProfileContext {

    private static final int TEMPLATE_DEFAULT_LINE_COUNT = 6;

    public PhoneConfiguration(BeanWithSettings device) {
        super(device);
    }

    public Map<String, Object> getContext() {
        Map context = super.getContext();
        context.put("lines", getLines());
        return context;
    }

    public Collection getLines() {
        PolycomPhone phone = (PolycomPhone) getDevice();
        int lineCount = Math.min(phone.getModel().getMaxLineCount(), TEMPLATE_DEFAULT_LINE_COUNT);
        ArrayList linesSettings = new ArrayList(lineCount);

        List<Line> lines = phone.getLines();
        for (Line line : lines) {
            linesSettings.add(line.getSettings());
        }

        // copy in blank lines of all unused lines
        for (int i = lines.size(); i < lineCount; i++) {
            Line line = phone.createLine();
            line.setPhone(phone);
            line.setPosition(i);
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }
}
