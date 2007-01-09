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

import java.io.File;

import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.vm.Voicemail;
import org.sipfoundry.sipxconfig.vm.VoicemailManager;

public class VoicemailSqueezer implements IPrimaryKeyConverter {
    private VoicemailManager m_manager;
    public VoicemailSqueezer(VoicemailManager manager) {
        m_manager = manager;
    }

    public Object getPrimaryKey(Object arg0) {
        Voicemail vm = (Voicemail) arg0;
        return vm.getPrimaryKey();
    }

    public Object getValue(Object arg0) {
        String[] ids = Voicemail.decodePrimaryKey(arg0);
        Voicemail vm = new Voicemail(new File(m_manager.getMailstoreDirectory()), ids[0], ids[1], ids[2]);
        return vm;
    }
}
