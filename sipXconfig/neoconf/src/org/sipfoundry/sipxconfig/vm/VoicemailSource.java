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
package org.sipfoundry.sipxconfig.vm;

import java.io.File;
import java.io.Serializable;

public class VoicemailSource {
    private File m_mailstore;
    
    public VoicemailSource(File mailstore) {
        m_mailstore = mailstore;
    }

    public Voicemail getVoicemail(Serializable voicemailId) {
        String[] ids = decodeVoicemailId(voicemailId);
        return new Voicemail(m_mailstore, ids[0], ids[1], ids[2]);
    }
    
    public Serializable getVoicemailId(Voicemail vm) {
        return vm.getUserId() + '/' + vm.getFolderId() + '/' + vm.getMessageId();        
    }

    public static String[] decodeVoicemailId(Object primaryKey) {
        String[] ids = primaryKey.toString().split(String.valueOf('/'));
        return ids;
    }
}
