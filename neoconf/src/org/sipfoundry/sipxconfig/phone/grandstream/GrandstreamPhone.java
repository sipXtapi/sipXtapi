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
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;

import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

/**
 * Support for Grandstream BudgeTone / HandyTone
 */
public class GrandstreamPhone extends Phone {
    
    public static final String BEAN_ID = "grandstream";

    public static final String SIP = "sip";

    public static final String EMPTY = "";

    public static final int KOLME = 3;

    private static final SettingFilter S_REALSETTINGS = new SettingFilter() {
            public boolean acceptSetting(Setting root, Setting setting) {
                boolean firstGeneration = setting.getParentPath().equals(root.getPath());
                boolean isLeaf = setting.getValues().isEmpty();            
                boolean isVirtual = (setting.getName().startsWith("_"));            
                return firstGeneration && isLeaf && !isVirtual;
            }
        };        

    private static final SettingFilter S_IPSETTINGS = new SettingFilter() {
            public boolean acceptSetting(Setting root_, Setting setting) {
                boolean isLeaf = setting.getValues().isEmpty();            
                boolean isBitmapped = (setting.getName().startsWith("__"));            
                return  isLeaf && isBitmapped;
            }
        };        
    public GrandstreamPhone() {
        super(BEAN_ID);
        init();
    }
    
    public GrandstreamPhone(GrandstreamModel model) {
        super(model); // sexy
        init();
    }
    
    private void init() {
        setPhoneTemplate("grandstream/grandstream.vm");        
    }

    public String getModelLabel() {
        return getModel().getLabel();
    }

    public String getPhoneFilename() {
        String phoneFilename = getSerialNumber();
        return getTftpRoot() + "/cfg" + phoneFilename.toLowerCase();
    }

    public Object getAdapter(Class c) {
        Object o = null;
        if (c == PhoneSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(c);
            adapter.setSetting(getSettings());
            adapter.addMapping(PhoneSettings.OUTBOUND_PROXY, "sip/P48");
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
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "port/P36");
            adapter.addMapping(LineSettings.USER_ID, "port/P35");            
            adapter.addMapping(LineSettings.PASSWORD, "port/P34");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "port/P3");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "sip/P47");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        
        return impl;
    }
    public Collection getProfileLines() {
        int lineCount = getModel().getMaxLineCount();
        ArrayList linesSettings = new ArrayList(lineCount);

        Collection lines = getLines();
        int i = 0;
        Iterator ilines = lines.iterator();
        for (; ilines.hasNext() && (i < lineCount); i++) {
            linesSettings.add(((Line) ilines.next()).getSettings());
        }

        // copy in blank lines of all unused lines
        for (; i < lineCount; i++) {
            Line line = createLine();
            line.setPosition(i);
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

    public void generateProfiles() {
        splitIpSettings();
        super.generateProfiles();
    }

    public Collection getRealSettings(Setting setting) {
        return SettingUtil.filter(S_REALSETTINGS, setting);
    }

    public Collection getIpSettings() {
        return SettingUtil.filter(S_IPSETTINGS, getSettings());
    }

    private void splitIpSettings() {
        Collection bitmaps = getIpSettings();
        Iterator bmi = bitmaps.iterator();
        while (bmi.hasNext()) {
            Setting bset = (Setting) bmi.next();
            String bname = bset.getName();
            int bpoint = bname.indexOf('-');

            if (bpoint < KOLME || bpoint == bname.length() - 1) {
                continue;
            }

            Setting btgt = getSettings().getSetting(bset.getParentPath().substring(1));
            
            int bofs = Integer.parseInt(bname.substring(bpoint + 1));
            
            String ipa = bset.getValue();
            String[] ipn = {EMPTY, EMPTY, EMPTY, EMPTY};

            if (!StringUtils.isBlank(ipa)) {
                ipn = ipa.split("\\.");
            }

            for (int i = 0; i < ipn.length; i++) {
                btgt.getSetting("P" + Integer.toString(bofs + i)).setValue(ipn[i]);
            }
        }
    }
}
