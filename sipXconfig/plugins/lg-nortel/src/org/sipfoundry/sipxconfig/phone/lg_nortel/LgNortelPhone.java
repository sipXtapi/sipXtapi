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
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import java.io.File;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;

public class LgNortelPhone extends Phone {
    public static final String BEAN_ID = "lg-nortel";

    public LgNortelPhone() {
        super(BEAN_ID);
    }

    public LgNortelPhone(LgNortelModel model) {
        super(model);
    }

    @Override
    public String getPhoneTemplate() {
        return "lg-nortel/mac.cfg.vm";
    }

    @Override
    public void initializeLine(Line line) {
        DeviceDefaults phoneDefaults = getPhoneContext().getPhoneDefaults();
        LgNortelLineDefaults defaults = new LgNortelLineDefaults(phoneDefaults, line);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initialize() {
        DeviceDefaults phoneDefaults = getPhoneContext().getPhoneDefaults();
        int lines = getLines().size();
        LgNortelPhoneDefaults defaults = new LgNortelPhoneDefaults(phoneDefaults, lines);
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public String getPhoneFilename() {
        return new File(getTftpRoot(), formatName("%s")).getPath();
    }

    private String formatName(String format) {
        // HACK: in some cases this function is called before serial number is assigned, it needs
        // to work with 'null' serial number
        String serialNumber = StringUtils.defaultString(getSerialNumber()).toUpperCase();
        return String.format(format, serialNumber);
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = LgNortelLineDefaults.getLineInfo(line);
        return lineInfo;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        LgNortelLineDefaults.setLineInfo(line, lineInfo);
    }
    
    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }

}
