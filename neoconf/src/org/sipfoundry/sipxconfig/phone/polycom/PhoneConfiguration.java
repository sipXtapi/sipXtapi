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

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineMetaData;

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

        Collection lines = phone.getLines();
        int i = 0;
        Iterator ilines = lines.iterator();
        for (; ilines.hasNext(); i++) {
            linesSettings.add(((Line) ilines.next()).getSettings());
        }

        // copy in blank lines of all unused lines
        for (; i < phone.getMaxLineCount(); i++) {
            PolycomLine line = new PolycomLine(getPhone(), new LineMetaData());
            line.getLineMetaData().setPosition(i);
            linesSettings.add(line.getSettings());
        }
        
        return linesSettings;
    }
}
