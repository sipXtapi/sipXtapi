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
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.io.CopyUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.phone.RestartException;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

/**
 * Support for Cisco ATA186/188 and Cisco 7905/7912
 */
public class CiscoAtaPhone extends CiscoPhone {

    public static final String BEAN_ID = "ciscoAta";

    private static final String ZERO = "0";

    private static final String IMAGE_ID = "imageid";

    private static final String NONE = "none";

    private static final int KOLME = 3;

    private static final String NOLLAX = "0x";

    private static final String ALLE = "_";

    private static final String PHONE_REGISTRATION_SETTING = "sip/Proxy";

    private static final String SIP_PORT_SETTING = "sip/SIPPort";
    
    private static final String UPGRADE_SETTING_GROUP = "upgrade";
    
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

    public CiscoAtaPhone() {
        super(BEAN_ID);
        init();
    }

    public CiscoAtaPhone(CiscoModel model) {
        super(model);
        init();
    }

    private void init() {
        setPhoneTemplate("cisco/cisco-ata.vm");
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
        return getPhoneContext().getSystemDirectory() + "/cisco/" + getCfgPrefix() + "-ptag.dat";
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
        return getPhoneContext().getSystemDirectory() + "/cisco/cfgfmt";
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
            String cmdline = MessageFormat.format("{0} {1} {2} {3}", cmd);
            LOG.info(cmdline);
            Process p = Runtime.getRuntime().exec(cmd);
            int errCode = p.waitFor();
            if (errCode != 0) {
                String msg = "Cisco profile conversion utility failed status code:" + errCode;
                StringWriter err = new StringWriter();
                err.write(msg.toCharArray());
                CopyUtils.copy(p.getErrorStream(), err);
                throw new RuntimeException(err.toString());
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
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
            adapter.addMapping(PhoneSettings.OUTBOUND_PROXY, "sip/SipOutBoundProxy");
            adapter.addMapping(PhoneSettings.OUTBOUND_PROXY_PORT, SIP_PORT_SETTING);
            adapter.addMapping(PhoneSettings.TFTP_SERVER, "network/TftpURL");
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
            // sip/SIPPort for outbound proxy?
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }

        return impl;
    }

    protected void defaultSettings() {
        super.defaultSettings();
        PhoneDefaults defaults = getPhoneContext().getPhoneDefaults();
        getSettings().getSetting(PHONE_REGISTRATION_SETTING).setValue(
                defaults.getRegistrationServer());
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
        if (getCfgPrefix().equals("ata")) {
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
            if (bttype.equals("integer")) {
                int btmp = Integer.decode(btgt.getValue()).intValue();
                btmp = btmp + (Integer.parseInt(bset.getValue()) << bofs);
                btgt.setValue(Integer.toString(btmp));
            } else {
                String btmp = btgt.getValue();
                if (btmp.startsWith(NOLLAX) || btmp.startsWith("0X")) {
                    long btx = Long.decode(btmp).longValue();
                    btx = btx + (Long.parseLong(bset.getValue()) << bofs);
                    btgt.setValue(NOLLAX + Long.toHexString(btx));
                } else {
                    btgt.setValue(btmp + bset.getValue());
                }
            }
        }
    }

    protected void sendCheckSyncToFirstLine() {
        if (getLines().size() == 0) {
            throw new RestartException("Restart command is sent to first line and "
                                       + "first phone line is not valid");
        }

        Line line = getLine(0);
        LineSettings settings = (LineSettings) line.getAdapter(LineSettings.class);

        getSipService().sendCheckSync(line.getUri(),
                                      getSettings().getSetting(PHONE_REGISTRATION_SETTING).getValue(),
                                      getSettings().getSetting(SIP_PORT_SETTING).getValue(),
                                      settings.getUserId());
    }
}
