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

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineData;
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Support for Grandstream BudgeTone / HandyTone
 */
public class GrandstreamPhone extends GenericPhone {

    public static final String SIP = "sip";

    public GrandstreamPhone() {
        setLineFactoryId(GrandstreamLine.FACTORY_ID);
        setPhoneTemplate("grandstream/grandstream.vm");
        setModelFile("grandstream/phone.xml");        
    }

    public Setting getSettingModel() {
        String sysPath = getPhoneContext().getSystemDirectory(); 
        File sysDir = new File(sysPath);
        File modelDefsFile = new File(sysDir, getModelFile());
        Setting all = new XmlModelBuilder(sysPath).buildModel(modelDefsFile);
        Setting model = all.getSetting(getModel().getModelId());
        
        return model;
    }
    
    public String getPhoneFilename() {
        String phoneFilename = getPhoneData().getSerialNumber();
        return getTftpRoot() + "/cfg" + phoneFilename.toUpperCase();
    }

    public GrandstreamModel getModel() {
        return GrandstreamModel.getModel(getPhoneData().getFactoryId());
    }

    public int getMaxLineCount() {
        return getModel().getMaxLines();
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

    public Collection getProfileLines() {
        ArrayList linesSettings = new ArrayList(getMaxLineCount());

        Collection lines = getLines();
        int i = 0;
        Iterator ilines = lines.iterator();
        for (; ilines.hasNext() && (i < getMaxLineCount()); i++) {
            linesSettings.add(((Line) ilines.next()).getSettings());
        }

        // copy in blank lines of all unused lines
        for (; i < getMaxLineCount(); i++) {
            GrandstreamLine line = new GrandstreamLine();
            line.setPhone(this);
            line.setLineData(new LineData());
            line.getLineData().setPosition(i);
            line.setDefaults(new PhoneDefaults());
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }
    
}
