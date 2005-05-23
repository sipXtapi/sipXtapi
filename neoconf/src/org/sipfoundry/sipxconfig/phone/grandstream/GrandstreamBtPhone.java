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
import org.sipfoundry.sipxconfig.phone.LineData;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Support for Grandstream BudgeTone, HandyTone
 */
public class GrandstreamBtPhone extends GrandstreamPhone {

    /** BEAN ACCESS ONLY */
    public GrandstreamBtPhone() {
        setLineFactoryId(GrandstreamBtLine.FACTORY_ID);
        setPhoneTemplate("grandstream/grandstream.vm");
        setModelFile("grandstream/phone.xml");        
    }

    // some settings should probably be added here
    protected void setDefaults(Setting settings) {
        String domainName = getPhoneContext().getDnsDomain();

        Setting pset = settings.getSetting(SIP);
        // FIXME: DnsDomain name is not the best value to use 
        // move code how polycom does it make avail in abstract phone class 
        pset.getSetting("P47").setValue(domainName);
        pset.getSetting("P48").setValue(domainName);
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
            GrandstreamBtLine line = new GrandstreamBtLine();
            line.setPhone(this);
            line.setLineData(new LineData());
            line.getLineData().setPosition(i);
            line.setUserId(StringUtils.EMPTY);
            line.setLoginId(StringUtils.EMPTY);
            line.setDisplayName(StringUtils.EMPTY);
            line.setPwd(StringUtils.EMPTY);
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

}
