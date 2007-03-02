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
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.SAXReader;
import org.dom4j.io.XMLWriter;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000 conference phone
 */
public class PolycomPhone extends Phone {
    public static final String BEAN_ID = "polycom";
    public static final String CALL = "call";
    static final String REGISTRATION_PATH = "reg/server/1/address";
    static final String REGISTRATION_PORT_PATH = "reg/server/1/port";
    private static final String CONTACT_MODE = "contact";
    private static final String DISPLAY_NAME_PATH = "reg/displayName";
    private static final String PASSWORD_PATH = "reg/auth.password";
    private static final String USER_ID_PATH = "reg/address";
    private static final String AUTHORIZATION_ID_PATH = "reg/auth.userId";

    private String m_phoneConfigDir = "polycom/mac-address.d";

    private String m_phoneTemplate = m_phoneConfigDir + "/phone.cfg.vm";

    private String m_sipTemplate = m_phoneConfigDir + "/sip-%s.cfg.vm";

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
        setDeviceVersion(PolycomModel.VER_2_0);
    }

    public String getDefaultVersionId() {
        DeviceVersion version = getDeviceVersion();
        return version != null ? version.getVersionId() : null;
    }

    /**
     * Default firmware version for polycom phones. Default is 1.6 right now
     * 
     * @param defaultVersionId 1.6 or 2.0
     */
    public void setDefaultVersionId(String defaultVersionId) {
        setDeviceVersion(DeviceVersion.getDeviceVersion(PolycomPhone.BEAN_ID + defaultVersionId));
    }

    @Override
    public void initialize() {
        PolycomPhoneDefaults phoneDefaults = new PolycomPhoneDefaults(getPhoneContext()
                .getPhoneDefaults());
        addDefaultBeanSettingHandler(phoneDefaults);

        PolycomIntercomDefaults intercomDefaults = new PolycomIntercomDefaults(this);
        addDefaultBeanSettingHandler(intercomDefaults);
    }

    @Override
    public void initializeLine(Line line) {
        PolycomLineDefaults lineDefaults = new PolycomLineDefaults(getPhoneContext()
                .getPhoneDefaults(), line);
        line.addDefaultBeanSettingHandler(lineDefaults);
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
        return String.format(m_sipTemplate, getDeviceVersion().getVersionId());
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
            Writer unformatted = new StringWriter();
            generateProfile(cfg, template, unformatted);
            out = new FileWriter(f);
            format(new StringReader(unformatted.toString()), out);
        } catch (IOException ioe) {
            throw new RuntimeException("Could not generate profile " + outputFile
                    + " from template " + template, ioe);
        } finally {
            if (out != null) {
                IOUtils.closeQuietly(out);
            }
        }
    }

    /**
     * Polycom 430 1.6.5 would not read files w/being formatted first. Unclear why.
     */
    static void format(Reader in, Writer wtr) {
        SAXReader xmlReader = new SAXReader();
        Document doc;
        try {
            doc = xmlReader.read(in);
        } catch (DocumentException e1) {
            throw new RuntimeException(e1);
        }
        OutputFormat pretty = OutputFormat.createPrettyPrint();
        XMLWriter xml = new XMLWriter(wtr, pretty);
        try {
            xml.write(doc);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    protected void setLineInfo(Line line, LineInfo externalLine) {
        line.setSettingValue(DISPLAY_NAME_PATH, externalLine.getDisplayName());
        line.setSettingValue(USER_ID_PATH, externalLine.getUserId());
        line.setSettingValue(PASSWORD_PATH, externalLine.getPassword());

        // Both userId and authId are required, see XCF-914
        line.setSettingValue(AUTHORIZATION_ID_PATH, externalLine.getUserId());

        line.setSettingValue(REGISTRATION_PATH, externalLine.getRegistrationServer());
        line.setSettingValue(REGISTRATION_PORT_PATH, externalLine.getRegistrationServerPort());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setUserId(line.getSettingValue(USER_ID_PATH));
        lineInfo.setDisplayName(line.getSettingValue(DISPLAY_NAME_PATH));
        lineInfo.setPassword(line.getSettingValue(PASSWORD_PATH));
        lineInfo.setRegistrationServer(line.getSettingValue(REGISTRATION_PATH));
        lineInfo.setRegistrationServerPort(line.getSettingValue(REGISTRATION_PORT_PATH));
        return lineInfo;
    }

    // XCF-668 Removed setting outbound proxy defaults
    // So by default, polycom phones will attempt to send sip traffic to server
    // it's registered with. Setting an outbound proxy to domain would be redundant
    // therefore unnec. and could potentially cause issues.
    public static class PolycomPhoneDefaults {
        private DeviceDefaults m_defaults;

        PolycomPhoneDefaults(DeviceDefaults defaults) {
            m_defaults = defaults;
        }

        private DeviceTimeZone getZone() {
            return m_defaults.getTimeZone();
        }

        @SettingEntry(path = "tcpIpApp.sntp/gmtOffset")
        public long getGmtOffset() {
            return getZone().getOffset();
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.enable")
        public boolean isDstEnabled() {
            return getZone().getDstOffset() != 0;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.fixedDayEnable")
        public boolean isFixedDayEnabled() {
            return isDstEnabled() && getZone().getStartDay() > 0;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.date")
        public int getStartDay() {
            if (!isDstEnabled()) {
                return 0;
            }
            if (isFixedDayEnabled()) {
                return getZone().getStartDay();
            }
            if (getZone().getStartWeek() == DeviceTimeZone.DST_LASTWEEK) {
                return 1;
            }

            return getZone().getStartWeek();
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.dayOfWeek.lastInMonth")
        public boolean isStartLastInMonth() {
            if (!isDstEnabled()) {
                return false;
            }
            if (isFixedDayEnabled()) {
                return true;
            }
            if (getZone().getStartWeek() == DeviceTimeZone.DST_LASTWEEK) {
                return true;
            }

            return false;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.date")
        public int getStopDate() {
            if (!isDstEnabled()) {
                return 0;
            }
            if (isFixedDayEnabled()) {
                return getZone().getStopDay();
            }
            if (getZone().getStopWeek() == DeviceTimeZone.DST_LASTWEEK) {
                return 1;
            }

            return getZone().getStopWeek();
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.dayOfWeek.lastInMonth")
        public boolean isStopDayOfWeekLastInMonth() {
            if (!isDstEnabled()) {
                return false;
            }
            if (isFixedDayEnabled()) {
                return false;
            }
            if (getZone().getStopWeek() == DeviceTimeZone.DST_LASTWEEK) {
                return true;
            }

            return false;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.dayOfWeek")
        public int getStartDayOfWeek() {
            return isDstEnabled() ? dayOfWeek(getZone().getStartDayOfWeek()) : 0;
        }
        
        static int dayOfWeek(int dayOfWeek) {
            // 1-based
            int dayOfWeekStartingOnMonday = ((dayOfWeek + 1) % 7) + 1;
            return dayOfWeekStartingOnMonday;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.month")
        public int getStartMonth() {
            return isDstEnabled() ? getZone().getStartMonth() : 0;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.start.time")
        public int getStartTime() {
            return isDstEnabled() ? getZone().getStartMonth() / 3600 : 0;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.dayOfWeek")
        public int getStopDayOfWeek() {
            return isDstEnabled() ? dayOfWeek(getZone().getStopDayOfWeek()) : 0;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.month")
        public int getStopMonth() {
            return isDstEnabled() ? getZone().getStopMonth() : 0;
        }

        @SettingEntry(path = "tcpIpApp.sntp/daylightSavings.stop.time")
        public int getStopTime() {
            return isDstEnabled() ? getZone().getStopTime() / 3600 : 0;
        }

        @SettingEntry(path = "voIpProt/server/1/address")
        public String getRegistrationServer() {
            return m_defaults.getDomainName();
        }
    }

    public static class PolycomLineDefaults {

        private DeviceDefaults m_defaults;
        private Line m_line;

        PolycomLineDefaults(DeviceDefaults defaults, Line line) {
            m_defaults = defaults;
            m_line = line;
        }
            
        @SettingEntry(paths = { "reg/server/1/transport", "reg/server/2/transport" })
        public String transport() {
            DeviceVersion ver = m_line.getPhone().getDeviceVersion();
            if (ver != null && "1.6".equals(ver.getVersionId())) {
                return "UDPonly";
            }    
            return "DNSnaptr";
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
            String mode = "disabled";
            User u = m_line.getUser();
            if (u != null) {
                mode = CONTACT_MODE;
            }

            return mode;
        }

        @SettingEntry(path = AUTHORIZATION_ID_PATH)
        public String getAuthorizationId() {
            return getAddress();
        }

        @SettingEntry(path = USER_ID_PATH)
        public String getAddress() {
            User u = m_line.getUser();
            if (u != null) {
                return u.getUserName();
            }
            return null;
        }

        @SettingEntry(path = PASSWORD_PATH)
        public String getAuthorizationPassword() {
            User u = m_line.getUser();
            if (u != null) {
                return u.getSipPassword();
            }
            return null;
        }

        @SettingEntry(path = DISPLAY_NAME_PATH)
        public String getDisplayName() {
            User u = m_line.getUser();
            if (u != null) {
                return u.getDisplayName();
            }
            return null;
        }

        @SettingEntry(path = REGISTRATION_PATH)
        public String getRegistrationServer() {
            User u = m_line.getUser();
            if (u != null) {
                return m_line.getPhoneContext().getPhoneDefaults().getDomainName();
            }
            return null;
        }
    }

    public void restart() {
        sendCheckSyncToFirstLine();
    }
}
