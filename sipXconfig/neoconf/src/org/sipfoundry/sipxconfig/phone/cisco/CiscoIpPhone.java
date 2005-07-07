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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineData;
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;

/**
 * Support for Cisco 7940/7960
 */
public class CiscoIpPhone extends CiscoPhone {

    /** BEAN ACCESS ONLY */
    public CiscoIpPhone() {
        setLineFactoryId(CiscoIpLine.FACTORY_ID);
        setPhoneTemplate("cisco/cisco-ip.vm");
        setModelFile("cisco/ip-phone.xml");        
    }
    
    public String getPhoneFilename() {
        String phoneFilename = getPhoneData().getSerialNumber();
        return getTftpRoot() + "/SIP" + phoneFilename.toUpperCase() + ".cnf";
    }

    public Object getAdapter(Class c) {
        Object o = null;
        if (c == PhoneSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(c);
            adapter.setSetting(getSettings());
            adapter.addMapping(PhoneSettings.OUTBOUND_PROXY, "sip/outbound_proxy");
            adapter.addMapping(PhoneSettings.OUTBOUND_PROXY_PORT, "sip/outbound_proxy_port");
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
            CiscoIpLine line = new CiscoIpLine();
            line.setPhone(this);
            line.setLineData(new LineData());
            line.getLineData().setPosition(i);
            line.setDefaults(new PhoneDefaults());
            linesSettings.add(line.getSettings());
            line.getSettings().getSetting("proxy/port").setValue(StringUtils.EMPTY);
        }

        return linesSettings;
    }
}
