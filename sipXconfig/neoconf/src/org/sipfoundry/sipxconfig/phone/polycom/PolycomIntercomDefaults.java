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
package org.sipfoundry.sipxconfig.phone.polycom;

import org.sipfoundry.sipxconfig.admin.intercom.Intercom;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Polycom allows many ring configurations to be set up using settings named
 * volpProt.SIP.alertInfo.x.value where x = 1, 2, ... n.
 * 
 * For now x = 1, always, because we only provide a single ring configuration.
 * 
 * Similarly there can be many ring classes. We rely on the default ring classes defined in
 * sip-1.6.cfg.vm and sip-2.0.cfg.vm rather than creating new ones.
 * 
 * See sections 4.6.1.1.3.2 "Alert Information <alertInfo/>" and 4.6.1.7.2 "Ring type <ringType/>"
 * in the Polycom admin manual.
 * 
 */
public class PolycomIntercomDefaults {
    static final int DEFAULT_RING_CLASS = 1;
    static final int AUTO_ANSWER_RING_CLASS = 3;
    static final int RING_ANSWER_RING_CLASS = 4;
    static final int DEFAULT_TIMEOUT = 2000;

    private Phone m_phone;

    public PolycomIntercomDefaults(Phone phone) {
        m_phone = phone;
    }

    @SettingEntry(path = "voIpProt.SIP/alertInfo/1/value")
    public String getAlertInfoValue() {
        Intercom intercom = getIntercom();
        if (intercom == null) {
            return null;
        }
        return intercom.getCode();
    }

    @SettingEntry(path = "voIpProt.SIP/alertInfo/1/class")
    public int getAlertInfoClass() {
        Intercom intercom = getIntercom();
        if (intercom == null) {
            return DEFAULT_RING_CLASS;
        }
        // If the timeout is zero, then auto-answer. Otherwise ring for
        // the specified timeout before auto-answering.
        int timeout = intercom.getTimeout();
        return timeout > 0 ? RING_ANSWER_RING_CLASS : AUTO_ANSWER_RING_CLASS;
    }

    @SettingEntry(path = "se/ringType/RING_ANSWER/timeout")
    public int getRingAnswerTimeout() {
        Intercom intercom = getIntercom();
        if (intercom == null || intercom.getTimeout() <= 0) {
            return DEFAULT_TIMEOUT;
        }
        return intercom.getTimeout();
    }

    protected Intercom getIntercom() {
        PhoneContext context = m_phone.getPhoneContext();
        Intercom intercom = context.getIntercomForPhone(m_phone);
        return intercom;
    }
}
