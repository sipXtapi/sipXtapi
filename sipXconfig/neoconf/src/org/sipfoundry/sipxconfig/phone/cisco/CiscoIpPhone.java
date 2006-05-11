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
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.setting.BeanValueStorage;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Support for Cisco 7940/7960
 */
public class CiscoIpPhone extends CiscoPhone {

    public static final String BEAN_ID = "ciscoIp";

    private static final String DST_AUTO_ADJUST = "dst_auto_adjust";

    private static final String ZEROMIN = ":00";
    
    private static final String SHORTNAME = "line/shortname";

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

    public void setPhoneContext(PhoneContext context) {
        super.setPhoneContext(context);
        
        CiscoIpDefaults defaults = new CiscoIpDefaults(context.getPhoneDefaults());
        BeanValueStorage vs = new BeanValueStorage(defaults);
        getSettingModel2().addSettingValueHandler(vs);
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
    
    public static class CiscoIpDefaults {
        private DeviceDefaults m_defaults;
        
        CiscoIpDefaults(DeviceDefaults defaults) {
            m_defaults = defaults;
        }
        
        private DeviceTimeZone getZone() {
            return m_defaults.getTimeZone();
        }

        @SettingEntry(path = "datetime/time_zone")
        public String getTimeZoneId() {
            return getZone().getShortName();
        }
        
        @SettingEntry(path = "datetime/dst_auto_adjust")
        public boolean isDstAutoAdjust() {
            return getZone().getDstOffset() != 0;
        }
        
        @SettingEntry(path = "datetime/dst_offset")
        public long getDstOffset() {
            return isDstAutoAdjust() ?  getZone().getDstOffset() / 3600 : 0;
        }

        @SettingEntry(path = "datetime/dst_start_day")
        public int getDstStartDay() {
            return isDstAutoAdjust() ? getZone().getStartDay() : 0;
        }
        
        @SettingEntry(path = "datetime/dst_start_day_of_week")
        public int getDstStartDayOfWeek() {
            return isDstAutoAdjust() ? getZone().getStartDayOfWeek() : 0;
        }
        
        @SettingEntry(path = "datetime/dst_start_month")
        public int getDstStartMonth() {
            return isDstAutoAdjust() ? getZone().getStartMonth() : 0;
        }
        
        @SettingEntry(path = "datetime/dst_start_time")
        public String getDstStartTime() {
            return  isDstAutoAdjust() ? time(getZone().getStartTime()) : null;
        }
        
        @SettingEntry(path = "datetime/dst_start_week_of_month")
        public int getDstStartWeekOfMonth() {
            return  isDstAutoAdjust() ? getZone().getStartWeek() : 0;
        }

        @SettingEntry(path = "datetime/dst_stop_day")
        public int getDstStopDay() {
            return isDstAutoAdjust() ? getZone().getStopDay() : 0;
        }
        
        @SettingEntry(path = "datetime/dst_stop_day_of_week")
        public int getDstStopDayOfWeek() {
            return isDstAutoAdjust() ? getZone().getStopDayOfWeek() : 0;
        }
        
        @SettingEntry(path = "datetime/dst_stop_month")
        public int getDstStopMonth() {
            return isDstAutoAdjust() ? getZone().getStopMonth() : 0;
        }
        
        @SettingEntry(path = "datetime/dst_stop_time")
        public String getStopTime() {
            return isDstAutoAdjust() ? time(getZone().getStopTime()) : null;            
        }
        
        @SettingEntry(path = "datetime/dst_stop_week_of_month")
        public String getStopWeekOfMonth() {
            return isDstAutoAdjust() ? time(getZone().getStopWeek()) : null;            
        }

        private String time(int time) {
            return String.valueOf(time / 3600) + ZEROMIN;
        }
        
    }

    protected void setDefaultTimeZone() {
        DeviceTimeZone mytz = new DeviceTimeZone();
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
            line.getSettings().getSetting(SHORTNAME).setValue(u.getUserName());
        }
    }
    
    public void addLine(Line line) {
        super.addLine(line);

        CiscoIpLineDefaults ls = new CiscoIpLineDefaults(line);
        BeanValueStorage bv = new BeanValueStorage(ls);
        line.getSettingModel2().addSettingValueHandler(bv);
    }
    
    class CiscoIpLineDefaults {
        private Line m_line;
        CiscoIpLineDefaults(Line line) {
            m_line = line;
        }
        
        @SettingEntry(path = SHORTNAME)
        public String getShortName() {
            String name = null;
            User u = m_line.getUser();
            if (u != null) {
                name = u.getUserName(); 
            }
            return name;
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
