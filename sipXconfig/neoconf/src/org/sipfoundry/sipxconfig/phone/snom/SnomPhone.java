/*
 *
 *
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 snom technology AG
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.snom;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;

public class SnomPhone extends Phone {

    public static final String BEAN_ID = "snom";

    public SnomPhone() {
        super(BEAN_ID);
        init();
    }

    public SnomPhone(SnomModel model) {
        super(model);
        init();
    }

    private void init() {
        setPhoneTemplate("snom/snom.vm");
    }

    public String getPhoneFilename() {
        String phoneFilename = getModel() + "-" + getSerialNumber().toUpperCase();
        return getWebDirectory() + "/" + phoneFilename + ".htm";
    }

    public int getMaxLineCount() {
        return getModel().getMaxLineCount();
    }

    public Object getAdapter(Class c) {
        Object o = null;
        if (c == PhoneSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(c);
            adapter.setSetting(getSettings());
            o = adapter.getImplementation();
        } else {
            o = super.getAdapter(c);
        }
        return o;
    }

    public Collection getProfileLines() {
        int lineCount = getModel().getMaxLineCount();
        ArrayList linesSettings = new ArrayList(getMaxLineCount());

        Collection lines = getLines();
        int i = 0;
        Iterator ilines = lines.iterator();
        for (; ilines.hasNext() && (i < lineCount); i++) {
            linesSettings.add(((Line) ilines.next()).getSettings());
        }

        for (; i < lineCount; i++) {
            Line line = createLine();
            line.setPosition(i);
            linesSettings.add(line.getSettings());
            LineSettings s = (LineSettings) line.getAdapter(LineSettings.class);
            s.setRegistrationServer(StringUtils.EMPTY);
            s.setRegistrationServerPort(StringUtils.EMPTY);
        }

        return linesSettings;
    }

    public Object getLineAdapter(Line line, Class interfac) {
        Object impl;
        if (interfac == LineSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(interfac);
            adapter.setSetting(line.getSettings());
            adapter.addMapping(LineSettings.USER_ID, "line/user_name");
            adapter.addMapping(LineSettings.PASSWORD, "line/user_pass");
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "line/user_pname");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "line/user_realname");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "line/user_host");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        return impl;
    }

    public void restart() {
        sendCheckSyncToFirstLine();        
    }
}
