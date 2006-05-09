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

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.phone.PhoneTimeZone;
import org.sipfoundry.sipxconfig.setting.BeanValueStorage;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000 conference phone
 */
public class PolycomPhone extends Phone {

    public static final String BEAN_ID = "polycom";

    public static final String CALL = "call";

    private static final String D_ENABLE = "daylightSavings.enable";

    private static final String D_FIXEDDAY = "daylightSavings.fixedDayEnable";

    private static final String D_STARTMONTH = "daylightSavings.start.month";

    private static final String D_STARTDATE = "daylightSavings.start.date";

    private static final String D_STARTTIME = "daylightSavings.start.time";

    private static final String D_STARTDOW = "daylightSavings.start.dayOfWeek";

    private static final String D_STARTLAST = "daylightSavings.start.dayOfWeek.lastInMonth";

    private static final String D_STOPMONTH = "daylightSavings.stop.month";

    private static final String D_STOPDATE = "daylightSavings.stop.date";

    private static final String D_STOPTIME = "daylightSavings.stop.time";

    private static final String D_STOPDOW = "daylightSavings.stop.dayOfWeek";

    private static final String D_STOPLAST = "daylightSavings.stop.dayOfWeek.lastInMonth";

    private static final String D_ZERO = "0";

    private static final String D_ONE = "1";
    
    private static final String CONTACT_MODE = "contact";

    private String m_phoneConfigDir = "polycom/mac-address.d";

    private String m_phoneTemplate = m_phoneConfigDir + "/phone.cfg.vm";

    private String m_sipTemplate = m_phoneConfigDir + "/sip.cfg.vm";

    private String m_coreTemplate = m_phoneConfigDir + "/ipmid.cfg.vm";

    private String m_applicationTemplate = "polycom/mac-address.cfg.vm";

    public PolycomPhone() {
        super(BEAN_ID);
        init();
    }

    public PolycomPhone(PolycomModel model) {
        super(model);
        init();
    }
    
    private void init() {        
        PolycomTimeZone ptz = new PolycomTimeZone(new PhoneTimeZone());
        BeanValueStorage bvs = new BeanValueStorage(ptz);
        getSettingModel2().addSettingValueHandler(bvs);
    }

    public String getPhoneTemplate() {
        return m_phoneTemplate;
    }

    public void setPhoneTemplate(String phoneTemplate) {
        m_phoneTemplate = phoneTemplate;
    }

    public String getCoreTemplate() {
        return m_coreTemplate;
    }

    public void setCoreTemplate(String coreTemplate) {
        m_coreTemplate = coreTemplate;
    }

    public String getApplicationTemplate() {
        return m_applicationTemplate;
    }

    public void setApplicationTemplate(String applicationTemplate) {
        m_applicationTemplate = applicationTemplate;
    }

    public String getSipTemplate() {
        return m_sipTemplate;
    }

    public void setSipTemplate(String sipTemplate) {
        m_sipTemplate = sipTemplate;
    }

    public void generateProfiles() {
        ApplicationConfiguration app = new ApplicationConfiguration(this);
        generateProfile(app, getApplicationTemplate(), app.getAppFilename());

        SipConfiguration sip = new SipConfiguration(this);
        generateProfile(sip, getSipTemplate(), app.getSipFilename());

        PhoneConfiguration phone = new PhoneConfiguration(this);
        generateProfile(phone, getPhoneTemplate(), app.getPhoneFilename());

        app.deleteStaleDirectories();
    }

    public void removeProfiles() {
        ApplicationConfiguration app = new ApplicationConfiguration(this);
        File cfgFile = new File(getTftpRoot(), app.getAppFilename());
        // new to call this function to generate stale directories list
        app.getDirectory();
        // this will remove all old directories
        app.deleteStaleDirectories();

        File[] files = {
            cfgFile
        };

        // and this will remove new ones
        VelocityProfileGenerator.removeProfileFiles(files);
    }

    /**
     * HACK: should be private, avoiding checkstyle error
     */
    void generateProfile(VelocityProfileGenerator cfg, String template, String outputFile) {
        FileWriter out = null;
        try {
            File f = new File(getTftpRoot(), outputFile);
            VelocityProfileGenerator.makeParentDirectory(f);
            out = new FileWriter(f);
            generateProfile(cfg, template, out);
        } catch (IOException ioe) {
            throw new RuntimeException("Could not generate profile " + outputFile
                    + " from template " + template, ioe);
        } finally {
            if (out != null) {
                IOUtils.closeQuietly(out);
            }
        }
    }

    public Object getAdapter(Class c) {
        Object o = null;
        if (c == PhoneSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(c);
            adapter.setSetting(getSettings());
            adapter.addMapping(PhoneSettings.DOMAIN_NAME, "voIpProt/server/1/address");

            // XCF-668 Polycom phones send sip traffic to server it's registered with
            // would only be non-traditional situations and hence not a good default
            // adapter.addMapping(PhoneSettings.OUTBOUND_PROXY,
            // "voIpProt/SIP.outboundProxy/address");
            // adapter.addMapping(PhoneSettings.OUTBOUND_PROXY_PORT,
            // "voIpProt/SIP.outboundProxy/port");
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
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "reg/auth.userId");
            adapter.addMapping(LineSettings.USER_ID, "reg/address");
            adapter.addMapping(LineSettings.PASSWORD, "reg/auth.password");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "reg/displayName");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "reg/server/1/address");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER_PORT, "reg/server/1/port");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }

        return impl;
    }
    
    protected void setDefaultTimeZone() {
        PhoneTimeZone mytz = new PhoneTimeZone();
        Setting datetime = getSettings().getSetting("tcpIpApp.sntp");

        datetime.getSetting("gmtOffset")
            .setValue(String.valueOf(mytz.getOffset()));
        
        if (mytz.getDstOffset() == 0) {
            datetime.getSetting(D_ENABLE).setValue(D_ZERO);
        } else {
            datetime.getSetting(D_ENABLE).setValue(D_ONE);

            if (mytz.getStartDay() > 0) {
                datetime.getSetting(D_FIXEDDAY).setValue(D_ONE);
                datetime.getSetting(D_STARTDATE)
                    .setValue(String.valueOf(mytz.getStartDay()));
                datetime.getSetting(D_STARTLAST).setValue(D_ZERO);
                datetime.getSetting(D_STOPDATE)
                    .setValue(String.valueOf(mytz.getStopDay()));
                datetime.getSetting(D_STOPLAST).setValue(D_ZERO);
            } else {
                datetime.getSetting(D_FIXEDDAY).setValue(D_ZERO);
                if (mytz.getStartWeek() == PhoneTimeZone.DST_LASTWEEK) {
                    datetime.getSetting(D_STARTDATE).setValue(D_ONE);
                    datetime.getSetting(D_STARTLAST).setValue(D_ONE);
                } else {
                    datetime.getSetting(D_STARTDATE)
                        .setValue(String.valueOf(mytz.getStartWeek()));
                    datetime.getSetting(D_STARTLAST).setValue(D_ZERO);
                }
                if (mytz.getStopWeek() == PhoneTimeZone.DST_LASTWEEK) {
                    datetime.getSetting(D_STOPDATE).setValue(D_ONE);
                    datetime.getSetting(D_STOPLAST).setValue(D_ONE);
                } else {
                    datetime.getSetting(D_STOPDATE)
                        .setValue(String.valueOf(mytz.getStopWeek()));
                    datetime.getSetting(D_STOPLAST).setValue(D_ZERO);
                }
            }
            datetime.getSetting(D_STARTDOW)
                .setValue(String.valueOf(mytz.getStartDayOfWeek()));
            datetime.getSetting(D_STARTMONTH)
                .setValue(String.valueOf(mytz.getStartMonth()));
            datetime.getSetting(D_STARTTIME)
                .setValue(String.valueOf(mytz.getStartTime() / 3600));

            datetime.getSetting(D_STOPDOW)
                .setValue(String.valueOf(mytz.getStopDayOfWeek()));
            datetime.getSetting(D_STOPMONTH)
                .setValue(String.valueOf(mytz.getStopMonth()));
            datetime.getSetting(D_STOPTIME)
                .setValue(String.valueOf(mytz.getStopTime() / 3600));
        }
    }

    protected void defaultLineSettings(Line line) {
        super.defaultLineSettings(line);

        User u = line.getUser();
        if (u != null) {
            DeviceDefaults defaults = getPhoneContext().getPhoneDefaults();
            Setting mwi = line.getSettings().getSetting("msg.mwi");
            String uri = u.getUserName() + '@' + defaults.getDomainName();
            mwi.getSetting("subscribe").setValue(uri);
            String voiceMail = defaults.getVoiceMail();
            mwi.getSetting("callBack").setValue(voiceMail + '@' + defaults.getDomainName());
            mwi.getSetting("callBackMode").setValue(CONTACT_MODE);
        }
    }
    public void addLine(Line line) {
        super.addLine(line);
        
        PolycomLineDefaults lineDefaults = new PolycomLineDefaults(getPhoneContext().getPhoneDefaults(), line);
        BeanValueStorage lineDefaultsValues = new BeanValueStorage(lineDefaults);
        line.getSettingModel2().addSettingValueHandler(lineDefaultsValues);
    }
    
    static class PolycomTimeZone {
        private PhoneTimeZone m_zone;
        
        PolycomTimeZone(PhoneTimeZone zone) {
            m_zone = zone;
        }
        
        @SettingEntry(path = "tcpIpApp.sntp/gmtOffset")
        public long getGmtOffset() {
            return m_zone.getOffset();
        }
        
        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.enable")
        public boolean isDstEnabled() {
            return m_zone.getDstOffset() != 0;
        }
        
        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.fixedDayEnable")
        public boolean isFixedDayEnabled() {
            return isDstEnabled() && m_zone.getStartDay() > 0;
        }
        
        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.date")
        public int getStartDay() {
            if (isFixedDayEnabled()) {
                return m_zone.getStartDay();
            }
            if (m_zone.getStartWeek() == PhoneTimeZone.DST_LASTWEEK) {
                return 1;
            }
            
            return m_zone.getStartWeek();            
        }
        
        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.dayOfWeek.lastInMonth")
        public boolean isStartLastInMonth() {
            if (isFixedDayEnabled()) {
                return true;
            }
            if (m_zone.getStartWeek() == PhoneTimeZone.DST_LASTWEEK) {
                return true;
            }
            
            return false;            
        }
        
        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.date")
        public int getStopDate() {
            if (isFixedDayEnabled()) {
                return m_zone.getStopDay();
            }
            if (m_zone.getStopWeek() == PhoneTimeZone.DST_LASTWEEK) {
                return 1;
            }
         
            return m_zone.getStopWeek();
        }
        
        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.dayOfWeek.lastInMonth")
        public boolean isStopDayOfWeekLastInMonth() {
            if (isFixedDayEnabled()) {
                return false;
            }            
            if (m_zone.getStopWeek() == PhoneTimeZone.DST_LASTWEEK) {
                return true;
            }
            
            return false;            
        }
        
        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.dayOfWeek")
        public int getStartDayOfWeek() {
            return isDstEnabled() ? m_zone.getStartDayOfWeek() : 0;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.month")
        public int getStartMonth() {
            return isDstEnabled() ? m_zone.getStartMonth() : 0;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.time")
        public int getStartTime() {
            return isDstEnabled() ? m_zone.getStartMonth() / 3600 : 0;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.dayOfWeek")
        public int getStopDayOfWeek() {
            return isDstEnabled() ? m_zone.getStopDayOfWeek() : 0;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.month")
        public int getStopMonth() {
            return isDstEnabled() ? m_zone.getStopMonth() : 0;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.time")
        public int getStopTime() {
            return isDstEnabled() ? m_zone.getStopTime() / 3600 : 0;
        }
    }
    
    static class PolycomLineDefaults {
            
        private DeviceDefaults m_defaults;
        private Line m_line;
        PolycomLineDefaults(DeviceDefaults defaults, Line line) {
            m_defaults = defaults;
            m_line = line;
        }
        
        @SettingEntry(path = "msg.mwi/subscribe")
        public String getMwiSubscribe() {
            String uri = null;
            User u = m_line.getUser();
            if (u != null) {
                uri = u.getUserName() + '@' + m_defaults.getDomainName();
            }
            
            return uri;
        }
        
        @SettingEntry(path = "msg.mwi/callBack")
        public String getCallBack() {
            String uri = null;
            User u = m_line.getUser();
            if (u != null) {
                uri = m_defaults.getVoiceMail() + '@' + m_defaults.getDomainName();
            }
            
            return uri;                    
        }
        
        @SettingEntry(path = "msg.mwi/callBackMode")
        public String getCallBackMode() {
            String mode = null;
            User u = m_line.getUser();
            if (u != null) {
                mode = CONTACT_MODE;
            }
            
            return mode;                                
        }
    }
    
    public void restart() {
        sendCheckSyncToFirstLine();
    }
}
