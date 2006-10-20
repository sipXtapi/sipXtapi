/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.clearone;

import java.io.File;

import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;

public class ClearonePhone extends Phone {
    public static final String BEAN_ID = "clearone";
    public static final String CONFIG_FILE = "C1MAXIP_%s.txt";
    public static final String DIALPLAN_FILE = "c1dialplan_%s.txt";
    
    public ClearonePhone() {
        super(BEAN_ID);
    }

    public ClearonePhone(ClearoneModel model) {
        super(model);
    }

    @Override
    public String getPhoneTemplate() {
        return "clearone/C1MAXIP.txt.vm";
    }

    public String getDialplanTemplate() {
        return "clearone/c1dialplan.txt.vm";
    }

    @Override
    public void initializeLine(Line line) {
        ClearoneLineDefaults defaults = new ClearoneLineDefaults(line, getPhoneContext()
                .getPhoneDefaults());
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initialize() {
        ClearonePhoneDefaults defaults = new ClearonePhoneDefaults(getPhoneContext()
                .getPhoneDefaults(), formatName(DIALPLAN_FILE));
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void generateProfiles() {
        super.generateProfiles();
        // generate some other files
        generateFile(getDialplanTemplate(), getDialplanFileName());
    }

    @Override
    public String getPhoneFilename() {
        return new File(getTftpRoot(), formatName(CONFIG_FILE)).getPath();
    }

    public String getDialplanFileName() {
        return new File(getTftpRoot(), formatName(DIALPLAN_FILE)).getPath();
    }

    private String formatName(String format) {
        String serialNumber = getSerialNumber().toUpperCase();
        return String.format(format, serialNumber);
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo info = new LineInfo();
        info.setDisplayName(line.getSettingValue(ClearoneLineDefaults.DISPLAY_NAME_SETTING));
        info.setUserId(line.getSettingValue(ClearoneLineDefaults.USER_ID_SETTING));
        info.setPassword(line.getSettingValue(ClearoneLineDefaults.PASSWORD_SETTING));
        // phone setting
        info.setRegistrationServer(getSettingValue(ClearoneLineDefaults.REGISTRATION_SERVER_SETTING));
        return info;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        line.setSettingValue(ClearoneLineDefaults.DISPLAY_NAME_SETTING, lineInfo.getDisplayName());
        line.setSettingValue(ClearoneLineDefaults.USER_ID_SETTING, lineInfo.getUserId());
        line.setSettingValue(ClearoneLineDefaults.PASSWORD_SETTING, lineInfo.getPassword());
        // phone setting
        setSettingValue(ClearoneLineDefaults.REGISTRATION_SERVER_SETTING, lineInfo.getRegistrationServer());
    }
}
