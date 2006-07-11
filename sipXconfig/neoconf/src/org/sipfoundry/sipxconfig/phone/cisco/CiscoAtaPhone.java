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

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collection;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.setting.SettingExpressionEvaluator;

/**
 * Support for Cisco ATA186/188 and Cisco 7905/7912
 */
public class CiscoAtaPhone extends CiscoPhone {
    public static final String BEAN_ID = "ciscoAta";
    private static final String ZERO = "0";
    private static final String TIMEZONE_SETTING = "service/TimeZone";
    private static final String REGISTRATION_ATA_PATH = "port/_Proxy.18x";
    private static final String REGISTRATION_PORT_ATA_PATH = "port/_ProxyPort.18x";
    private static final String REGISTRATION_PATH = "port/_Proxy.79";
    private static final String REGISTRATION_PORT_PATH = "port/_ProxyPort.79";
    private static final String DISPLAY_NAME_PATH = "port/DisplayName";
    private static final String TFTP_PATH = "network/TftpURL";
    private static final String USER_ID_PATH = "port/UID";
    private static final String LOGIN_ID_PATH = "port/LoginID";
    private static final String PASSWORD_PATH = "port/PWD";
    private static final String VOICEMAIL_PATH = "caller/VoiceMailNumber";
    private static final Log LOG = LogFactory.getLog(CiscoAtaPhone.class);

    private String m_ptagDat;

    private String m_cfgfmtUtility;

    private String m_binDir;

    private boolean m_isTextFormatEnabled;

    public CiscoAtaPhone() {
        super(BEAN_ID);
    }

    public CiscoAtaPhone(CiscoModel model) {
        super(model);
    }

    @Override
    public Setting loadSettings() {
        return loadDynamicSettings("phone.xml");
    }

    @Override
    public Setting loadLineSettings() {
        return loadDynamicSettings("line.xml");
    }

    private Setting loadDynamicSettings(String basename) {
        SettingExpressionEvaluator evaluator = new CiscoAtaSettingExpressionEvaluator(getModel()
                .getModelId());
        return getModelFilesContext().loadDynamicModelFile(basename, getModel().getBeanId(),
                evaluator);
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new CiscoAtaDefaults(getPhoneContext().getPhoneDefaults()));
    }

    @Override
    public void initializeLine(Line line) {
        line.addDefaultBeanSettingHandler(new CiscoAtaLineDefaults(line));
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setDisplayName(line.getSettingValue(DISPLAY_NAME_PATH));
        lineInfo.setUserId(line.getSettingValue(USER_ID_PATH));
        if (getModel() == CiscoModel.MODEL_ATA18X) {
            lineInfo.setRegistrationServer(line.getSettingValue(REGISTRATION_ATA_PATH));
            lineInfo.setRegistrationServerPort(line.getSettingValue(REGISTRATION_PORT_ATA_PATH));
        } else {
            lineInfo.setRegistrationServer(line.getSettingValue(REGISTRATION_PATH));
            lineInfo.setRegistrationServerPort(line.getSettingValue(REGISTRATION_PORT_PATH));
        }
        return lineInfo;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        line.setSettingValue(DISPLAY_NAME_PATH, lineInfo.getDisplayName());
        line.setSettingValue(USER_ID_PATH, lineInfo.getUserId());
        if (getModel() == CiscoModel.MODEL_ATA18X) {
            line.setSettingValue(REGISTRATION_ATA_PATH, lineInfo.getRegistrationServer());
            line.setSettingValue(REGISTRATION_PORT_ATA_PATH, lineInfo
                            .getRegistrationServerPort());
        } else {            
            line.setSettingValue(REGISTRATION_PATH, lineInfo.getRegistrationServer());
            line.setSettingValue(REGISTRATION_PORT_PATH, lineInfo.getRegistrationServerPort());
        }
    }

    /**
     * Generate files in text format. Won't be usable by phone, but you can use cisco config tool
     * to convert manually. This is mostly for debugging
     * 
     * @param isTextFormatEnabled true to save as text, default is false
     */
    public void setTextFormatEnabled(boolean isTextFormatEnabled) {
        m_isTextFormatEnabled = isTextFormatEnabled;
    }

    public String getPhoneFilename() {
        String phoneFilename = getSerialNumber();
        return getTftpRoot() + '/' + getCiscoModel().getCfgPrefix() + phoneFilename.toLowerCase();
    }

    public void setPtagDat(String ptagDat) {
        m_ptagDat = ptagDat;
    }

    public String getPtagDat() {
        if (m_ptagDat != null) {
            return m_ptagDat;
        }
        return getPhoneContext().getSystemDirectory() + "/ciscoAta/"
                + getCiscoModel().getCfgPrefix() + "-ptag.dat";
    }

    public void setCfgfmtUtility(String cfgfmtUtility) {
        m_cfgfmtUtility = cfgfmtUtility;
    }

    /**
     * this points to the cfgfmt utility in etc/cisco directory
     */
    public String getCfgfmtUtility() {
        if (m_cfgfmtUtility != null) {
            return m_cfgfmtUtility;
        }
        return getPhoneContext().getSystemDirectory() + "/ciscoAta/cfgfmt";
    }

    public String getBinDir() {
        return m_binDir;
    }

    public void setBinDir(String binDir) {
        m_binDir = binDir;
    }

    public void generateProfile(Writer wtr) {
        CiscoAtaProfileWriter pwtr = new CiscoAtaProfileWriter(wtr);
        pwtr.write(this);
    }

    public void generateProfiles() {
        String outputfile = getPhoneFilename();
        String outputTxtfile = outputfile + ".txt";
        FileWriter wtr = null;

        try {
            wtr = new FileWriter(outputTxtfile);
            generateProfile(wtr);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(wtr);
        }

        requireFile(getCfgfmtUtility());
        requireFile(getPtagDat());
        try {
            String[] cmd = {
                getCfgfmtUtility(), "-t" + getPtagDat(), outputTxtfile, outputfile
            };
            LOG.info(StringUtils.join(cmd, ' '));
            Process p = Runtime.getRuntime().exec(cmd);
            int errCode = p.waitFor();
            if (errCode != 0) {
                String msg = "Cisco profile conversion utility failed status code:" + errCode;
                StringWriter err = new StringWriter();
                err.write(msg.toCharArray());
                IOUtils.copy(p.getErrorStream(), err);
                throw new RuntimeException(err.toString());
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
        if (!m_isTextFormatEnabled) {
            try {
                FileUtils.forceDelete(new File(outputTxtfile));
            } catch (IOException e) {
                // ignore delete failure
                LOG.info(e.getMessage());
            }
        }
    }

    private void requireFile(String filename) {
        File f = new File(filename);
        if (!f.exists()) {
            StringBuffer msg = new StringBuffer();

            msg.append("Cannot complete configuration of Cisco device ");
            msg.append(getSerialNumber());
            msg.append(".  Required file cannot be found ");
            msg.append(f.getAbsolutePath());
            msg.append(". This file is supplied by Cisco support.");
            throw new RuntimeException(msg.toString());
        }
    }

    public class CiscoAtaLineDefaults {
        private Line m_line;

        CiscoAtaLineDefaults(Line line) {
            m_line = line;
        }

        @SettingEntry(
            paths = {
                USER_ID_PATH, LOGIN_ID_PATH
                })
        public String getUserName() {
            String userId = null;
            User u = m_line.getUser();
            if (u != null) {
                userId = u.getUserName();
            }
            return userId;
        }

        @SettingEntry(path = PASSWORD_PATH)
        public String getPassword() {
            String password = null;
            User u = m_line.getUser();
            if (u != null) {
                password = u.getSipPassword();
            }
            return password;
        }

        @SettingEntry(path = DISPLAY_NAME_PATH)
        public String getDisplayName() {
            String displayName = null;
            User u = m_line.getUser();
            if (u != null) {
                displayName = u.getDisplayName();
            }
            return displayName;
        }

        @SettingEntry(
            paths = {
                REGISTRATION_PATH, REGISTRATION_ATA_PATH
                })
        public String getRegistrationServer() {
            return m_line.getPhoneContext().getPhoneDefaults().getDomainName();
        }
    }

    public class CiscoAtaDefaults {
        private DeviceDefaults m_defaults;

        CiscoAtaDefaults(DeviceDefaults defaults) {
            m_defaults = defaults;
        }

        @SettingEntry(path = TIMEZONE_SETTING)
        public int getTimeZoneOffset() {
            int tzmin = m_defaults.getTimeZone().getOffsetWithDst() / 60;
            int atatz;

            if (tzmin % 60 == 0) {
                atatz = tzmin / 60;
                if (atatz < 0) {
                    atatz += 25;
                }
            } else {
                atatz = tzmin;
            }

            return atatz;
        }

        @SettingEntry(path = TFTP_PATH)
        public String getTftpServer() {
            return m_defaults.getTftpServer();
        }

        @SettingEntry(path = VOICEMAIL_PATH)
        public String getVoiceMailNumber() {
            return m_defaults.getVoiceMail();
        }
    }

    public Collection<Line> getProfileLines() {
        ArrayList<Line> lines = new ArrayList(getMaxLineCount());

        lines.addAll(getLines());

        // copy in blank lines of all unused lines
        for (int i = lines.size(); i < getMaxLineCount(); i++) {
            Line line = createLine();
            line.setPhone(this);
            line.setPosition(i);
            lines.add(line);
            line.initialize();
            line.addDefaultBeanSettingHandler(new StubAtaLine());
        }

        return lines;
    }

    public static class StubAtaLine {

        @SettingEntry(
            paths = {
                DISPLAY_NAME_PATH, USER_ID_PATH, LOGIN_ID_PATH, USER_ID_PATH, PASSWORD_PATH
                })
        public String getZero() {
            return ZERO;
        }
    }

    static class CiscoAtaSettingExpressionEvaluator implements SettingExpressionEvaluator {
        private String m_model;

        public CiscoAtaSettingExpressionEvaluator(String model) {
            m_model = model;
        }

        public boolean isExpressionTrue(String expression, Setting setting_) {
            return m_model.matches(expression);
        }
    }
}
