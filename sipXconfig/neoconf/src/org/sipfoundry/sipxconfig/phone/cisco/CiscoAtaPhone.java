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
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.phone.PhoneTimeZone;
import org.sipfoundry.sipxconfig.setting.BeanValueStorage;
import org.sipfoundry.sipxconfig.setting.ConditionalSet;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.setting.SettingExpressionEvaluator;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.sipfoundry.sipxconfig.setting.SettingValue2;
import org.sipfoundry.sipxconfig.setting.SettingValueHandler;
import org.sipfoundry.sipxconfig.setting.SettingValueImpl;

/**
 * Support for Cisco ATA186/188 and Cisco 7905/7912
 */
public class CiscoAtaPhone extends CiscoPhone {

    public static final String BEAN_ID = "ciscoAta";

    private static final String ZERO = "0";

    private static final String IMAGE_ID = "imageid";

    private static final String ATA_ID = "ata";

    private static final String NONE = "none";

    private static final int KOLME = 3;

    private static final String NOLLAX = "0x";

    private static final String NOLLAX_UPPER = "0X";
    
    private static final String INTEGER_SETTING = "integer";

    private static final String ALLE = "_";

    private static final String UPGRADE_SETTING_GROUP = "upgrade";

    private static final String TIMEZONE_SETTING = "service/TimeZone";

    private static final Log LOG = LogFactory.getLog(CiscoAtaPhone.class);

    private static final SettingFilter S_REALGROUPS = new SettingFilter() {
        public boolean acceptSetting(Setting root_, Setting setting) {
            boolean isLeaf = setting.getValues().isEmpty();
            boolean isVirtual = (setting.getName().startsWith(ALLE));
            return !isLeaf && !isVirtual;
        }
    };

    private static final SettingFilter S_REALSETTINGS = new SettingFilter() {
        public boolean acceptSetting(Setting root, Setting setting) {
            boolean firstGeneration = setting.getParentPath().equals(root.getPath());
            boolean isLeaf = setting.getValues().isEmpty();
            boolean isVirtual = (setting.getName().startsWith(ALLE));
            return firstGeneration && isLeaf && !isVirtual;
        }
    };

    private static final SettingFilter S_BITMAPSETTINGS = new SettingFilter() {
        public boolean acceptSetting(Setting root_, Setting setting) {
            boolean isLeaf = setting.getValues().isEmpty();
            boolean isBitmapped = (setting.getName().startsWith("__"));
            return isLeaf && isBitmapped;
        }
    };

    private String m_ptagDat;

    private String m_cfgfmtUtility;

    private String m_binDir;

    private boolean m_isTextFormatEnabled;

    public CiscoAtaPhone() {
        super(BEAN_ID);
        init();
    }

    public CiscoAtaPhone(CiscoModel model) {
        super(model);
        init();
    }

    private void init() {
        setPhoneTemplate("ciscoAta/cisco-ata.vm");

        CiscoAtaTimeZone tz = new CiscoAtaTimeZone(new PhoneTimeZone());
        BeanValueStorage vs = new BeanValueStorage(tz);
        getSettingModel2().addSettingValueHandler(vs);
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
    
    public void setSettings(Setting settings) {
        super.setSettings(settings);
        
        Collection bitmaps = getBitmapSettings();
        PackBitmaps bitmapHandler = new PackBitmaps();
        bitmapHandler.preprocess(settings, bitmaps);        
        getSettingModel2().addSettingValueHandler(bitmapHandler);
    }

    public String getCfgPrefix() {
        CiscoModel model = (CiscoModel) getModel();
        return model.getCfgPrefix();
    }

    public String getPhoneFilename() {
        String phoneFilename = getSerialNumber();
        return getTftpRoot() + '/' + getCfgPrefix() + phoneFilename.toLowerCase();
    }

    public void setPtagDat(String ptagDat) {
        m_ptagDat = ptagDat;
    }

    public String getPtagDat() {
        if (m_ptagDat != null) {
            return m_ptagDat;
        }
        return getPhoneContext().getSystemDirectory() + "/ciscoAta/" + getCfgPrefix()
                + "-ptag.dat";
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

    public void generateProfiles() {
        String outputfile = getPhoneFilename();
        String outputTxtfile = outputfile + ".txt";
        FileWriter wtr = null;

        packBitmaps();

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

    public Object getAdapter(Class c) {
        Object o = null;
        if (c == PhoneSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(c);
            adapter.setSetting(getSettings());
            adapter.addMapping(PhoneSettings.TFTP_SERVER, "network/TftpURL");
            if (!getCfgPrefix().equals(ATA_ID)) {
                adapter.addMapping(PhoneSettings.VOICE_MAIL_NUMBER, "caller/VoiceMailNumber");
            }
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
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "port/LoginID");
            adapter.addMapping(LineSettings.USER_ID, "port/UID");
            adapter.addMapping(LineSettings.PASSWORD, "port/PWD");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "port/DisplayName");
            if (getCfgPrefix().equals(ATA_ID)) {
                adapter.addMapping(LineSettings.REGISTRATION_SERVER, "port/_Proxy.18x");
                adapter.addMapping(LineSettings.REGISTRATION_SERVER_PORT, "port/_ProxyPort.18x");
            } else {
                adapter.addMapping(LineSettings.REGISTRATION_SERVER, "port/_Proxy.79");
                adapter.addMapping(LineSettings.REGISTRATION_SERVER_PORT, "port/_ProxyPort.79");
            }
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }

        return impl;
    }

    protected void setDefaultTimeZone() {
        PhoneTimeZone mytz = new PhoneTimeZone();

        int tzmin = mytz.getOffsetWithDst() / 60;
        int atatz;

        if (tzmin % 60 == 0) {
            atatz = tzmin / 60;
            if (atatz < 0) {
                atatz += 25;
            }
        } else {
            atatz = tzmin;
        }
        getSettings().getSetting(TIMEZONE_SETTING).setValue(String.valueOf(atatz));
    }
    
    public class CiscoAtaTimeZone {
        private PhoneTimeZone m_zone;
        
        CiscoAtaTimeZone(PhoneTimeZone zone) {
            m_zone = zone;
        }
        @SettingEntry(path = TIMEZONE_SETTING)
        public int getTimeZoneOffset() {
            int tzmin = m_zone.getOffsetWithDst() / 60;
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
    }

    public String getSoftwareUpgradeConfig() {
        Setting swupgrade = getSettings().getSetting(UPGRADE_SETTING_GROUP);

        if (swupgrade == null) {
            return StringUtils.EMPTY;
        }

        String swimage = swupgrade.getSetting("upgradecode." + getCfgPrefix()).getValue();
        String imageid = swupgrade.getSetting(IMAGE_ID + "." + getCfgPrefix()).getValue();
        CiscoModel model = (CiscoModel) getModel();
        String upghex = model.getUpgCode();

        if (StringUtils.isBlank(swimage) || swimage.equals(NONE) || imageid.equals(ZERO)) {
            return StringUtils.EMPTY;
        }

        return "upgradecode:3," + upghex + ",0.0.0.0,69," + imageid + "," + swimage;
    }

    public String getLogoUpgradeConfig() {
        if (getCfgPrefix().equals(ATA_ID)) {
            return StringUtils.EMPTY;
        }

        Setting logoupgrade = getSettings().getSetting(UPGRADE_SETTING_GROUP);

        if (logoupgrade == null) {
            return StringUtils.EMPTY;
        }

        String logofile = logoupgrade.getSetting("logofile").getValue();
        String imageid = logoupgrade.getSetting("logoid").getValue();

        if (StringUtils.isBlank(logofile) || logofile.equals(NONE) || imageid.equals(ZERO)) {
            return StringUtils.EMPTY;
        }

        return "upgradelogo:" + imageid + ",0," + logofile;
    }

    public String getProxyConfig() {
        if (getLines().size() == 0) {
            return StringUtils.EMPTY;
        }

        Line line = getLine(0);
        LineSettings settings = (LineSettings) line.getAdapter(LineSettings.class);
        if (settings == null) {
            return StringUtils.EMPTY;
        }

        return "Proxy:" + settings.getRegistrationServer() + ":"
                + settings.getRegistrationServerPort();
    }

    public Collection getProfileLines() {
        ArrayList linesSettings = new ArrayList(getMaxLineCount());

        Collection lines = getLines();
        int i = 0;
        Iterator ilines = lines.iterator();
        for (; ilines.hasNext() && (i < getMaxLineCount()); i++) {
            Line line = (Line) ilines.next();
            linesSettings.add(line.getSettings());
        }

        // copy in blank lines of all unused lines
        for (; i < getMaxLineCount(); i++) {
            Line line = createLine();
            line.setPhone(this);
            line.setPosition(i);
            LineSettings settings = (LineSettings) line.getAdapter(LineSettings.class);
            settings.setDisplayName(ZERO);
            settings.setUserId(ZERO);
            settings.setAuthorizationId(ZERO);
            settings.setDisplayName(ZERO);
            settings.setPassword(ZERO);
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

    public Collection getRealSettingGroups() {
        return SettingUtil.filter(S_REALGROUPS, getSettings());
    }

    public Collection getRealSettings(Setting setting) {
        return SettingUtil.filter(S_REALSETTINGS, setting);
    }

    public Collection getBitmapSettings() {
        return SettingUtil.filter(S_BITMAPSETTINGS, getSettings());
    }

    // FIXME : Write a settinghandler that checks by name '__' and returns accordingly
    static class PackBitmaps implements SettingValueHandler {
        private Map<String, SettingValue2> m_bitmapTargets = new HashMap();
        
        void preprocess(Setting settings, Collection bitmaps) {
            Iterator bmi = bitmaps.iterator();
            while (bmi.hasNext()) {
                Setting bset = (Setting) bmi.next();
                String bname = bset.getName();
                int bpoint = bname.indexOf('.');

                if (bpoint < KOLME || bpoint == bname.length() - 1) {
                    continue;
                }

                String tgtname = bname.substring(2, bpoint);
                String btpath = bset.getParentPath() + Setting.PATH_DELIM + tgtname;
                Setting btgt = settings.getSetting(btpath.substring(1));

                int bofs = Integer.parseInt(bname.substring(bpoint + 1));

                String bttype = btgt.getType().getName();
                if (bttype.equals(INTEGER_SETTING)) {
                    int btmp = Integer.decode(btgt.getValue()).intValue();
                    btmp = btmp + (Integer.parseInt(bset.getValue()) << bofs);
                    SettingValue2 value = new SettingValueImpl(Integer.toString(btmp));
                    m_bitmapTargets.put(btgt.getPath(), value);
                } else {
                    String btmp = btgt.getValue();
                    if (btmp.startsWith(NOLLAX) || btmp.startsWith(NOLLAX_UPPER)) {
                        long btx = Long.decode(btmp).longValue();
                        btx = btx + (Long.parseLong(bset.getValue()) << bofs);
                        SettingValue2 value = new SettingValueImpl(NOLLAX + Long.toHexString(btx));
                        m_bitmapTargets.put(btgt.getPath(), value);
                    } else {
                        SettingValue2 value = new SettingValueImpl(btmp + bset.getValue());
                        m_bitmapTargets.put(btgt.getPath(), value);
                    }
                }
            }            
        }
        
        public SettingValue2 getSettingValue(Setting setting) {
            return m_bitmapTargets.get(setting.getPath());
        }        
    }
    public void packBitmaps() {
        Collection bitmaps = getBitmapSettings();
        Iterator bmi = bitmaps.iterator();
        while (bmi.hasNext()) {
            Setting bset = (Setting) bmi.next();
            String bname = bset.getName();
            int bpoint = bname.indexOf('.');

            if (bpoint < KOLME || bpoint == bname.length() - 1) {
                continue;
            }

            String tgtname = bname.substring(2, bpoint);
            String btpath = bset.getParentPath() + Setting.PATH_DELIM + tgtname;
            Setting btgt = getSettings().getSetting(btpath.substring(1));

            int bofs = Integer.parseInt(bname.substring(bpoint + 1));

            String bttype = btgt.getType().getName();
            if (bttype.equals(INTEGER_SETTING)) {
                int btmp = Integer.decode(btgt.getValue()).intValue();
                btmp = btmp + (Integer.parseInt(bset.getValue()) << bofs);
                btgt.setValue(Integer.toString(btmp));
            } else {
                String btmp = btgt.getValue();
                if (btmp.startsWith(NOLLAX) || btmp.startsWith(NOLLAX_UPPER)) {
                    long btx = Long.decode(btmp).longValue();
                    btx = btx + (Long.parseLong(bset.getValue()) << bofs);
                    btgt.setValue(NOLLAX + Long.toHexString(btx));
                } else {
                    btgt.setValue(btmp + bset.getValue());
                }
            }
        }
    }

    public Setting evaluateModel(ConditionalSet conditional) {
        CiscoAtaSettingExpressionEvaluator gssee = new CiscoAtaSettingExpressionEvaluator(
                getModel().getModelId());
        Setting model = conditional.evaluate(gssee);
        return model;
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
