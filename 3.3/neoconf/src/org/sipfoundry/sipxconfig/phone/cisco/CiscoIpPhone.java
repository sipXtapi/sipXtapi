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
package org.sipfoundry.sipxconfig.phone.cisco;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.phone.PhoneTimeZone;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;

/**
 * Support for Cisco 7940/7960
 */
public class CiscoIpPhone extends CiscoPhone {

    public static final String BEAN_ID = "ciscoIp";

    private static final String DST_AUTO_ADJUST = "dst_auto_adjust";

    private static final String ZEROMIN = ":00";

    public CiscoIpPhone() {
        super(BEAN_ID);
        init();
    }

    public CiscoIpPhone(CiscoModel model) {
        super(model);
        init();
    }

    private void init() {
        setPhoneTemplate("ciscoIp/cisco-ip.vm");
    }

    public String getPhoneFilename() {
        String phoneFilename = getSerialNumber();
        return getTftpRoot() + "/SIP" + phoneFilename.toUpperCase() + ".cnf";
    }

    public Object getAdapter(Class c) {
        Object o = null;
        if (c == PhoneSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(c);
            adapter.setSetting(getSettings());
            // XCF-760 safe not to set these, traffic is sent to registration
            // server by default, FQDN or just DN. 
            // adapter.addMapping(PhoneSettings.OUTBOUND_PROXY, "sip/outbound_proxy");
            // adapter.addMapping(PhoneSettings.OUTBOUND_PROXY_PORT, "sip/outbound_proxy_port");
            adapter.addMapping(PhoneSettings.VOICE_MAIL_NUMBER, "phone/messages_uri");

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
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "line/authname");
            adapter.addMapping(LineSettings.USER_ID, "line/name");
            adapter.addMapping(LineSettings.PASSWORD, "line/password");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "line/displayname");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "proxy/address");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER_PORT, "proxy/port");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }

        return impl;
    }

    protected void setDefaultTimeZone() {
        PhoneTimeZone mytz = new PhoneTimeZone();
        Setting datetime = getSettings().getSetting("datetime");

        datetime.getSetting("time_zone").setValue(mytz.getShortName());
        
        if (mytz.getDstOffset() == 0) {
            datetime.getSetting(DST_AUTO_ADJUST).setValue("0");
        } else {
            datetime.getSetting(DST_AUTO_ADJUST).setValue("1");
            datetime.getSetting("dst_offset")
                .setValue(String.valueOf(mytz.getDstOffset() / 3600));

            datetime.getSetting("dst_start_day")
                .setValue(String.valueOf(mytz.getStartDay()));
            datetime.getSetting("dst_start_day_of_week")
                .setValue(String.valueOf(mytz.getStartDayOfWeek()));
            datetime.getSetting("dst_start_month")
                .setValue(String.valueOf(mytz.getStartMonth()));
            datetime.getSetting("dst_start_time")
                .setValue(String.valueOf(mytz.getStartTime() / 3600) + ZEROMIN);
            datetime.getSetting("dst_start_week_of_month")
                .setValue(String.valueOf(mytz.getStartWeek()));

            datetime.getSetting("dst_stop_day")
                .setValue(String.valueOf(mytz.getStopDay()));
            datetime.getSetting("dst_stop_day_of_week")
                .setValue(String.valueOf(mytz.getStopDayOfWeek()));
            datetime.getSetting("dst_stop_month")
                .setValue(String.valueOf(mytz.getStopMonth()));
            datetime.getSetting("dst_stop_time")
                .setValue(String.valueOf(mytz.getStopTime() / 3600) + ZEROMIN);
            datetime.getSetting("dst_stop_week_of_month")
                .setValue(String.valueOf(mytz.getStopWeek()));
        }
    }
    
    public void defaultLineSettings(Line line) {
        super.defaultLineSettings(line);

        User u = line.getUser();
        if (u != null) {
            line.getSettings().getSetting("line/shortname").setValue(u.getUserName());
        }
    }

    public Collection getProfileLines() {
        ArrayList linesSettings = new ArrayList(getMaxLineCount());

        Collection lines = getLines();
        int i = 0;
        Iterator ilines = lines.iterator();
        for (; ilines.hasNext() && (i < getMaxLineCount()); i++) {
            linesSettings.add(((Line) ilines.next()).getSettings());
        }

        // copy in blank lines of all unused lines
        for (; i < getMaxLineCount(); i++) {
            Line line = createLine();
            line.setPosition(i);
            linesSettings.add(line.getSettings());
            LineSettings s = (LineSettings) line.getAdapter(LineSettings.class);
            s.setRegistrationServer(StringUtils.EMPTY);
            s.setRegistrationServerPort(StringUtils.EMPTY);
        }

        return linesSettings;
    }
}
