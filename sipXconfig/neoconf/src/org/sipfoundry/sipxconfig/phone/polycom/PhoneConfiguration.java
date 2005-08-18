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
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.VelocityProfileGenerator;

/**
 * Responsible for generating MAC_ADDRESS.d/phone.cfg
 */
public class PhoneConfiguration extends VelocityProfileGenerator {

    public PhoneConfiguration(PolycomPhone phone) {
        super(phone);
    }

    protected void addContext(VelocityContext context) {
        super.addContext(context);
        context.put("lines", getLines());
    }

    public Collection getLines() {
        PolycomPhone phone = (PolycomPhone) getPhone();
        int lineCount = phone.getModel().getMaxLineCount();
        ArrayList linesSettings = new ArrayList(lineCount);

        Collection lines = phone.getLines();
        int i = 0;
        Iterator ilines = lines.iterator();
        for (; ilines.hasNext(); i++) {
            linesSettings.add(((Line) ilines.next()).getSettings());
        }

        // copy in blank lines of all unused lines
        for (; i < lineCount; i++) {
            Line line = phone.createLine();
            line.setPhone(phone);
            line.setPosition(i);
            linesSettings.add(line.getSettings());
            LineSettings s = (LineSettings) line.getAdapter(LineSettings.class);
            s.setRegistrationServer(StringUtils.EMPTY);
            s.setRegistrationServerPort(StringUtils.EMPTY);
        }
        
        return linesSettings;
    }
}
