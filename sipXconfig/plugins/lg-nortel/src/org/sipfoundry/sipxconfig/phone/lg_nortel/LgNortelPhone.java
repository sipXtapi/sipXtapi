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
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

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
        LgNortelLineDefaults defaults = new LgNortelLineDefaults(line, getPhoneContext()
                .getPhoneDefaults());
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initialize() {
        PhoneContext phoneContext = getPhoneContext();
        LgNortelPhoneDefaults defaults = new LgNortelPhoneDefaults(phoneContext
                .getPhoneDefaults());
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public String getPhoneFilename() {
        return new File(getTftpRoot(), formatName("%s.cfg")).getPath();
    }

    private String formatName(String format) {
        // HACK: in some cases this function is called before serial number is assigned, it needs
        // to work with 'null' serial number
        String serialNumber = StringUtils.defaultString(getSerialNumber()).toUpperCase();
        return String.format(format, serialNumber);
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        return LgNortelLineDefaults.getLineInfo(this, line);
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        LgNortelLineDefaults.setLineInfo(this, line, lineInfo);
    }
}
