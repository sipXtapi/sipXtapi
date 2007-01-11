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
package org.sipfoundry.sipxconfig.site.vm;

import java.io.Serializable;

import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.vm.Voicemail;
import org.sipfoundry.sipxconfig.vm.VoicemailSource;

public class VoicemailSqueezer implements IPrimaryKeyConverter {
    private VoicemailSource m_source;
    public VoicemailSqueezer(VoicemailSource source) {
        m_source = source;
    }

    public Object getPrimaryKey(Object arg0) {
        Voicemail vm = (Voicemail) arg0;
        return m_source.getVoicemailId(vm);
    }

    public Object getValue(Object arg0) {
        Voicemail vm = m_source.getVoicemail((Serializable) arg0); 
        return vm;
    }
}
