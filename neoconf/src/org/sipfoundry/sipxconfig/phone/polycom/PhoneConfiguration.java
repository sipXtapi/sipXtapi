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

import org.apache.velocity.VelocityContext;

/**
 * Responsible for generating MAC_ADDRESS.d/phone.cfg
 */
public class PhoneConfiguration extends ConfigurationFile {

    public PhoneConfiguration(PolycomPhone phone) {
        super(phone);
    }

    protected void addContext(VelocityContext context) {
        super.addContext(context);
        context.put("lines", getLines());
    }

    public Collection getLines() {
        PolycomPhone phone = getPhone();
        ArrayList linesSettings = new ArrayList(phone.getMaxLineCount());

        int n = phone.getLineCount();
        int i = 0;
        for (; i < n; i++) {
            linesSettings.add(phone.getLine(i).getSettings());
        }

        // copy in blank lines of all unused lines
        for (; i < phone.getMaxLineCount(); i++) {
            PolycomLine line = new PolycomLine(getPhone());
            line.getLineMetaData().setPosition(i);
            linesSettings.add(line.getSettings());
        }
        
        return linesSettings;
    }
}
