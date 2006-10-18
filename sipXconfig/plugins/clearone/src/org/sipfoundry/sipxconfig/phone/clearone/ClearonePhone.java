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

import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;

public class ClearonePhone extends Phone {
    public static final String BEAN_ID = "clearone";

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

    @Override
    public void initializeLine(Line line) {
        ClearoneLineDefaults defaults = new ClearoneLineDefaults(line, getPhoneContext()
                .getPhoneDefaults());
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initialize() {
        ClearonePhoneDefaults defaults = new ClearonePhoneDefaults(getPhoneContext()
                .getPhoneDefaults());
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void generateProfiles() {
        super.generateProfiles();
        // generate some other files
    }

    @Override
    public String getPhoneFilename() {
        String serialNumber = getSerialNumber();
        return getTftpRoot() + "/C1MAXIP_" + serialNumber + ".txt";
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo info = new LineInfo();
        return info;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
    }
}
