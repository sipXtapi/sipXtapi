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
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;

public class SnomPhone extends Phone {

    public static final String BEAN_ID = "snom";
    
    public static final String USER_HOST = "line/user_host";
    
    public static final String MAILBOX = "line/user_mailbox";
    
    public static final String OUTBOUND_PROXY = "sip/user_outbound";

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
        return getWebDirectory() + "/" + getProfileName();
    }

    String getProfileName() {
        StringBuffer buffer = new StringBuffer(getModel().getName());
        String serialNumber = getSerialNumber();
        if (StringUtils.isNotBlank(serialNumber)) {
            buffer.append("-");
            buffer.append(serialNumber.toUpperCase());
        }
        buffer.append(".htm");
        return buffer.toString();
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

    protected void defaultSettings() {
        super.defaultSettings();

        Setting settings = getSettings();
        PhoneDefaults defaults = getPhoneContext().getPhoneDefaults();
        String configUrl = defaults.getProfileRootUrl() + '/' + getProfileName();
        settings.getSetting("update/setting_server").setValue(configUrl);
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
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, USER_HOST);
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        return impl;
    }
    
    protected void defaultLineSettings(Line line) {
        super.defaultLineSettings(line);
        
        PhoneDefaults defaults = getPhoneContext().getPhoneDefaults();
        // registration server shouldn't be used, proxy(e.g domain name) should handle delivery
        String domainName = defaults.getDomainName();
        String registrationUri = domainName + ";transport=udp";
        line.getSettings().getSetting(USER_HOST).setValue(registrationUri);
        User user = line.getUser(); 
        if (user != null) {  
            // XCF-722 Setting this to the mailbox (e.g. 101) would fix issue
            // where mailbox button on phone calls voicemail server, but would
            // break MWI subscription because SUBSCRIBE message fails 
            // authentication unless this value is user's username
            line.getSettings().getSetting(MAILBOX).setValue(user.getUserName());

            // XPB-398 This forces TCP, this is defined in conjunction with "transport=udp"
            // This is benign w/o SRV, but required w/SRV
            line.getSettings().getSetting(OUTBOUND_PROXY).setValue(defaults.getFullyQualifiedDomainName());
        }
    }    
        
    public void restart() {
        sendCheckSyncToFirstLine();
    }
}
