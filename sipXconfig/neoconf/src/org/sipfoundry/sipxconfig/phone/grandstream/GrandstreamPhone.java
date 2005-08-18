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
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;

/**
 * Support for Grandstream BudgeTone / HandyTone
 */
public class GrandstreamPhone extends Phone {
    
    public static final String BEAN_ID = "grandstream";

    public static final String SIP = "sip";

    public GrandstreamPhone() {
        super(BEAN_ID);
        init();
    }
    
    public GrandstreamPhone(GrandstreamModel model) {
        super(model); // sexy
        init();
    }
    
    private void init() {
        setPhoneTemplate("grandstream/grandstream.vm");        
    }

    public String getModelLabel() {
        return getModel().getLabel();
    }

    public String getPhoneFilename() {
        String phoneFilename = getSerialNumber();
        return getTftpRoot() + "/cfg" + phoneFilename.toUpperCase();
    }

    public Object getAdapter(Class c) {
        Object o = null;
        if (c == PhoneSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(c);
            adapter.setSetting(getSettings());
            adapter.addMapping(PhoneSettings.OUTBOUND_PROXY, "sip/P48");
            o = adapter.getImplementation();
        } else {
            o = super.getAdapter(c);
        }
        
        return o;
    }

    public Object getLineAdapter(Line line, Class interfac) {
        Object impl;
        if (interfac == LineSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(interfac);
            adapter.setSetting(line.getSettings());
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "port/P36");
            adapter.addMapping(LineSettings.USER_ID, "port/P35");            
            adapter.addMapping(LineSettings.PASSWORD, "port/P34");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "port/P3");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "sip/P47");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        
        return impl;
    }
    public Collection getProfileLines() {
        int lineCount = getModel().getMaxLineCount();
        ArrayList linesSettings = new ArrayList(lineCount);

        Collection lines = getLines();
        int i = 0;
        Iterator ilines = lines.iterator();
        for (; ilines.hasNext() && (i < lineCount); i++) {
            linesSettings.add(((Line) ilines.next()).getSettings());
        }

        // copy in blank lines of all unused lines
        for (; i < lineCount; i++) {
            Line line = createLine();
            line.setPosition(i);
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }
    
}
