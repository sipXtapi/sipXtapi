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
package org.sipfoundry.sipxconfig.phone.kphone;

import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.BeanValueStorage;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class KPhone extends Phone {    
    public static final PhoneModel MODEL_KPHONE = new PhoneModel("kphone", "KPhone");
    
    private static final String REG_URI = "Registration/SipUri";    
    
    public KPhone() {
        super(MODEL_KPHONE);
        setPhoneTemplate("kphone/kphonerc.vm");
    }
    
    public String getPhoneFilename() {
        return getWebDirectory() + "/" + getSerialNumber() + ".kphonerc";
    }    
    
    public Object getLineAdapter(Line line, Class interfac) {
        Object impl;
        if (interfac == LineSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(interfac);
            adapter.setSetting(line.getSettings());
            adapter.addMapping(LineSettings.USER_ID, "Registration/UserName");            
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "Registration/SipServer");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        
        return impl;        
    }
    
    public void defaultLineSettings(Line line) {
        super.defaultLineSettings(line);
        
        line.getSettings().getSetting(REG_URI).setValue(line.getUri());
    }
    
    public void addLine(Line line) {
        super.addLine(line);
        
        KPhoneLineDefaults defaults = new KPhoneLineDefaults(line);
        BeanValueStorage vs = new BeanValueStorage(defaults);
        line.getSettingModel2().addSettingValueHandler(vs);
    }
    
    static class KPhoneLineDefaults {
        private Line m_line;
        public KPhoneLineDefaults(Line line) {
            m_line = line;
        }
        
        @SettingEntry(path = REG_URI)
        public String getUri() {
            return m_line.getUri();
        }
    }
}
