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

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.phone.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000 conference phone
 */
public class PolycomPhone extends Phone {
    
    public static final String BEAN_ID = "polycom";

    public static final String CALL = "call";

    private static final String VOICEMAIL_EXT = "101";

    private String m_phoneConfigDir = "polycom/mac-address.d";

    private String m_phoneTemplate = m_phoneConfigDir + "/phone.cfg.vm";

    private String m_sipTemplate = m_phoneConfigDir + "/sip.cfg.vm";

    private String m_coreTemplate = m_phoneConfigDir + "/ipmid.cfg.vm";

    private String m_applicationTemplate = "polycom/mac-address.cfg.vm";
    
    public PolycomPhone() {
        super(BEAN_ID);
    }
    
    public PolycomPhone(PolycomModel model) {
        super(model);
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
        return m_sipTemplate;
    }

    public void setSipTemplate(String sipTemplate) {
        m_sipTemplate = sipTemplate;
    }

    public void generateProfiles() {
        ApplicationConfiguration app = new ApplicationConfiguration(this);
        generateProfile(app, getApplicationTemplate(), app.getAppFilename());

        CoreConfiguration core = new CoreConfiguration(this);
        generateProfile(core, getCoreTemplate(), app.getCoreFilename());

        PhoneConfiguration phone = new PhoneConfiguration(this);
        generateProfile(phone, getPhoneTemplate(), app.getPhoneFilename());

        VelocityProfileGenerator sip = new VelocityProfileGenerator(this);
        generateProfile(sip, getSipTemplate(), app.getSipFilename());

        app.deleteStaleDirectories();
    }
    
    /**
     * HACK: should be private, avoiding checkstyle error
     */
    void generateProfile(VelocityProfileGenerator cfg, String template, String outputFile) {
        FileWriter out = null;
        try {
            File f = new File(getTftpRoot(), outputFile);
            makeParentDirectory(f);
            out = new FileWriter(f);
            generateProfile(cfg, template, out);
        } catch (IOException ioe) {
            throw new RuntimeException("Could not generate profile " + outputFile + " from template " + template, ioe);
        } finally {
            if (out != null) {
                IOUtils.closeQuietly(out);
            }
        }
    }

    public Object getAdapter(Class c) {
        Object o = null;
        if (c == PhoneSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(c);
            adapter.setSetting(getSettings());
            adapter.addMapping(PhoneSettings.DOMAIN_NAME, "voIpProt/server/1/address");
            adapter.addMapping(PhoneSettings.OUTBOUND_PROXY, "voIpProt/SIP.outboundProxy/address");
            adapter.addMapping(PhoneSettings.OUTBOUND_PROXY_PORT, "voIpProt/SIP.outboundProxy/port");
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
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "reg/auth.userId");
            adapter.addMapping(LineSettings.USER_ID, "reg/address");            
            adapter.addMapping(LineSettings.PASSWORD, "reg/auth.password");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "reg/displayName");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "reg/server/1/address");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER_PORT, "reg/server/1/port");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        
        return impl;
    }
    
    protected void defaultLineSettings(Line line) {
        super.defaultLineSettings(line);
        
        User u = line.getUser();
        if (u != null) {
            PhoneDefaults defaults = getPhoneContext().getPhoneDefaults();
            Setting mwi = line.getSettings().getSetting("msg.mwi");
            String uri = u.getUserName() + '@' + defaults.getDomainName();
            mwi.getSetting("subscribe").setValue(uri);
            mwi.getSetting("callBack").setValue(VOICEMAIL_EXT + '@' + defaults.getDomainName());
            mwi.getSetting("callBackMode").setValue("contact");
        }        
    }

}

