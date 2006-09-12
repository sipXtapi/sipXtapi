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
package org.sipfoundry.sipxconfig.phone.acme;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Example phone.
 */
public class AcmePhone extends Phone {
    public static final PhoneModel MODEL_ACME = new PhoneModel("acmePhone", "Generic Phone");

    private static final String USER_ID_SETTING = "credential/userId";
    private static final String DISPLAY_NAME_SETTING = "credential/displayName";
    private static final String PASSWORD_SETTING = "credential/password";
    private static final String TFTP_SERVER_SETTING = "server/tftpServer";
    private static final String REGISTRATION_SERVER_SETTING = "server/registrationServer";
    private static final String REGISTRATION_SERVER_PORT_SETTING = "server/registrationServerPort";
    
    public AcmePhone() {
        super(MODEL_ACME);
    }
    
    /**
     * Each subclass must decide how as much of this generic line information translates
     * into its own setting model.
     */
    @Override
    protected void setLineInfo(Line line, LineInfo info) {
        line.setSettingValue(USER_ID_SETTING, info.getUserId());
        line.setSettingValue(DISPLAY_NAME_SETTING, info.getDisplayName());
        line.setSettingValue(PASSWORD_SETTING, info.getPassword());
        line.setSettingValue(REGISTRATION_SERVER_SETTING, info.getRegistrationServer());
        line.setSettingValue(REGISTRATION_SERVER_PORT_SETTING, info.getRegistrationServerPort());
    }
    
    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("phone.xml", Phone.MODEL.getBeanId());        
    }
    
    @Override
    protected Setting loadLineSettings() {
        return getModelFilesContext().loadModelFile("line.xml", Phone.MODEL.getBeanId());        
    }

    /**
     * Each subclass must decide how as much of this generic line information can be contructed
     * from its own setting model.
     */
    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo info = new LineInfo();
        info.setDisplayName(line.getSettingValue(DISPLAY_NAME_SETTING));
        info.setUserId(line.getSettingValue(USER_ID_SETTING));
        info.setPassword(line.getSettingValue(PASSWORD_SETTING));
        info.setRegistrationServer(line.getSettingValue(REGISTRATION_SERVER_SETTING));
        info.setRegistrationServerPort(line.getSettingValue(REGISTRATION_SERVER_PORT_SETTING));
        return info;
    }
    
    
    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new AcmeDefaults(getPhoneContext()));
    }
    
    @Override
    public void initializeLine(Line line) {
        line.addDefaultBeanSettingHandler(new AcmeLineDefaults(line));
    }
    
    public static class AcmeDefaults {
        private PhoneContext m_context;
        AcmeDefaults(PhoneContext context) {
            m_context = context;
        }
        
        @SettingEntry(path = TFTP_SERVER_SETTING)
        public String getProfileDirectory() {
            return m_context.getPhoneDefaults().getTftpServer();            
        }        
    }
    
    public static class AcmeLineDefaults {
        private Line m_line;        
        AcmeLineDefaults(Line line) {
            m_line = line;
        }
        
        @SettingEntry(path = USER_ID_SETTING)
        public String getUserName() {
            String userName = null;
            User user = m_line.getUser();
            if (user != null) {
                userName = user.getUserName();
            }
            return userName;            
        }
        
        @SettingEntry(path = DISPLAY_NAME_SETTING)
        public String getDisplayName() {
            String displayName = null;
            User user = m_line.getUser();
            if (user != null) {
                displayName = user.getDisplayName();
            }
            return displayName;            
        }
        
        @SettingEntry(path = PASSWORD_SETTING)
        public String getPassword() {
            String password = null;
            User user = m_line.getUser();
            if (user != null) {
                password = user.getSipPassword();
            }
            return password;
        }
     
        @SettingEntry(path = REGISTRATION_SERVER_SETTING)
        public String getRegistrationServer() {
            DeviceDefaults defaults = m_line.getPhoneContext().getPhoneDefaults();
            return defaults.getDomainName();
        }        
    }    
}
